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

float HIGH_TEMP = 75;
// SHOULD BE 60 AND 50
float DESIRED_MAX_TEMP = 63;
float DESIRED_MIN_TEMP = 48; 

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
  int pirData = digitalRead(PIR_PIN);           // Read PIR sensor data

  char payload[30];
  sprintf(payload, "%.2f || %d || %d", temperature, irData, pirData);
  client.publish(MQTT_TOPIC, payload);

  startSystem(temperature, irData, pirData);
}

void activateCoolingSystem()
{
  digitalWrite(RELAY_PIN, HIGH);
}

void deactivateCoolingSystem()
{
  digitalWrite(RELAY_PIN, LOW);
}

void activateBuzzer() {
  // Play buzzer
  tone(BUZZER_PIN, 1000);
  delay(5000);
}

void deactivateBuzzer() {
    noTone(BUZZER_PIN);
}

void startSystem(float temp, int irValue, int pirValue) 
{ 
  if (pirValue == LOW) 
  { 
    digitalWrite(WHITE_LED_PIN, LOW);
  } 
  else 
  { // Cup presence
    digitalWrite(WHITE_LED_PIN, HIGH);

    if (temp >= HIGH_TEMP) { // more than 75 degrees Celsius
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      activateCoolingSystem();
      if (irValue == HIGH) {
        digitalWrite(WARNING_LED_PIN, HIGH);
        Serial.println("Hand Motion detected");
      }
      else{
        digitalWrite(WARNING_LED_PIN, LOW);
        Serial.println("Motion stopped");
      }
    } 
    else if (temp >= DESIRED_MAX_TEMP && temp < HIGH_TEMP) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, LOW);
      activateCoolingSystem();
      if (irValue == HIGH) {
        digitalWrite(WARNING_LED_PIN, HIGH);
        Serial.println("Hand Motion detected");
      }
      else{
        digitalWrite(WARNING_LED_PIN, LOW);
        Serial.println("Motion stopped");
      }
    } 
    else if (temp >= DESIRED_MIN_TEMP && temp < DESIRED_MAX_TEMP) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, HIGH);
      digitalWrite(BLUE_LED_PIN, LOW);
      deactivateCoolingSystem();
      activateBuzzer();
      if (irValue == HIGH) {
        Serial.println("Hand Motion detected");
        deactivateBuzzer();
      }
    } else if (temp < DESIRED_MIN_TEMP) {
      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(YELLOW_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);
      digitalWrite(BLUE_LED_PIN, HIGH);
    }
  }
}