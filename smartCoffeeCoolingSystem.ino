#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

const char *WIFI_SSID = "Joexuen"; //your WiFi SSID
const char *WIFI_PASSWORD = "leejoexuen"; // your password
const char *MQTT_SERVER = "34.134.222.220"; // your VM instance public IP address
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "iot"; // MQTT topic

const int DHT_PIN = A4;
const int DHT_TYPE = DHT11;
const int RED_LED_PIN = 38;
const int YELLOW_LED_PIN = 39;
const int GREEN_LED_PIN = 40;
const int BLUE_LED_PIN = 21;
const int WHITE_LED_PIN = 14;
const int RELAY_PIN = 48;
const int WARNING_LED_PIN = 47;  
const int IR_PIN = A2;
const int PIR_PIN = 42;
const int BUZZER_PIN = 12; 

float HIGH_TEMP = 28;
// SHOULD BE 60 AND 50
float DESIRED_MAX_TEMP = 20;
float DESIRED_MIN_TEMP = 10; 

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(WARNING_LED_PIN, OUTPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("Connected to MQTT server");
    }
    else
    {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  delay(5000);

  float temperature = dht.readTemperature();    // Read temperature from DHT11 Sensor
  int irData = !digitalRead(IR_PIN);             // Read IR sensor data
  int pirData = !digitalRead(PIR_PIN);           // Read PIR sensor data

  char payload[30];
  sprintf(payload, "%.2f || %d || %d", temperature, irData, pirData);
  client.publish(MQTT_TOPIC, payload);

  // Check the temperature of coffee
  indicateTemperature(temperature);

  // Check if a cup is detected using IR sensor
  // Check if the drink is high temperature
  if (irData == HIGH)
  {
    // Indicate the presence of coffee cup on coaster
    digitalWrite(WHITE_LED_PIN, HIGH);       

    // Indicate human presence
    if(pirData == HIGH)                      
    {
      // Warning Orange LED ON to notify user
      digitalWrite(WARNING_LED_PIN, HIGH);  
      tone(BUZZER_PIN, 5000);
      Serial.println("Motion Detected");
    }
    else
    {
      digitalWrite(WARNING_LED_PIN, LOW);
      noTone(BUZZER_PIN);
      Serial.println("Motion not Detected");
    }
  }
  else
  {
    digitalWrite(WHITE_LED_PIN, LOW);
  }
}

void notifyUser()
{
  //play buzzer
  tone(BUZZER_PIN, 1000);
  delay(5000);
  noTone(BUZZER_PIN);
}

void activateCoolingSystem()
{
  digitalWrite(RELAY_PIN, HIGH);
}

void deactivateCoolingSystem()
{
  digitalWrite(RELAY_PIN, LOW);
}

void indicateTemperature(float temperature)
{
  // TEMPERATURE IS HIGHER THAN OR EQUAL TO 80
  if (temperature >= HIGH_TEMP)
  {
    digitalWrite(RED_LED_PIN, HIGH);    // Red LED ON
    digitalWrite(YELLOW_LED_PIN, LOW);  // Yellow LED OFF
    digitalWrite(GREEN_LED_PIN, LOW);   // Green LED OFF
    digitalWrite(BLUE_LED_PIN, LOW);    // Blue LED OFF
    activateCoolingSystem();            // Turn on the fan
  }

  //TEMPERATURE IS HIGHER THAN 60 OR LOWER THAN 80
  if (temperature > DESIRED_MAX_TEMP && temperature < HIGH_TEMP)
  {
    digitalWrite(RED_LED_PIN, LOW);      // Red LED OFF
    digitalWrite(YELLOW_LED_PIN, HIGH);  // Yellow LED ON
    digitalWrite(GREEN_LED_PIN, LOW);    // Green LED OFF
    digitalWrite(BLUE_LED_PIN, LOW);     // Blue LED OFF
    activateCoolingSystem();             // Turn on the fan
  }
  //TEMPERATURE IS BETWEEN 50-60
  else if (temperature >= DESIRED_MIN_TEMP && temperature <= DESIRED_MAX_TEMP)
  {
    digitalWrite(RED_LED_PIN, LOW);     // Red LED OFF
    digitalWrite(YELLOW_LED_PIN, LOW);  // Yellow LED OFF
    digitalWrite(GREEN_LED_PIN, HIGH);  // Green LED ON
    digitalWrite(BLUE_LED_PIN, LOW);    // Blue LED OFF
    deactivateCoolingSystem();          // Turn off the fan
    //notifyUser();
  }
  else
  {
    digitalWrite(RED_LED_PIN, LOW);     // Red LED OFF
    digitalWrite(YELLOW_LED_PIN, LOW);  // Yellow LED OFF
    digitalWrite(GREEN_LED_PIN, LOW);   // Green LED OFF
    digitalWrite(BLUE_LED_PIN, HIGH);   // Blue LED ON
    deactivateCoolingSystem();          // Turn off the fan
  }
}