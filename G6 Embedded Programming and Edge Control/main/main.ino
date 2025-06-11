#include <WiFi.h>
#include "SensorModule.h"
#include "ActuatorModule.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

// WIFI Configuration
const char *SSID = "Cynex@2.4GHz";
const char *PASSWORD = "cyber@cynex";
const String SERVER_URL = "https://test-server-owq2.onrender.com";
const String USER_ID = "5ZyZcYb6wpdlObgeJiaGZ0ydMbW2";

// Adafruit IO MQTT config
const char* MQTT_SERVER = "io.adafruit.com";
const int MQTT_PORT = 1883;
const char* MQTT_TOPIC = "SmartGrow/feeds/actuator-status";

// GPIO Definitions
const uint8_t SOIL_PINS[] = {34, 35, 36, 39};
const char *PLANT_IDS[] = {
    "plant_JU4Rj78DEHUM2lNYHzd3",
    "plant_CyhF06FW5a1KTmCvM0zf",
    "plant_hG7bPH7Np9WXtDe1zBVE",
    "plant_iKjnBJcBaGTx6LyCXUy2"
  };

// Actuator PIN
const int PUMP_PIN = 1;
const int FAN_PIN = 2;
const int LIGHT_PIN = 4;


// WiFi & MQTT Clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Sensor Setup
SensorModule sensor(DHT_PIN, DHT_TYPE, (uint8_t *)SOIL_PINS, 4);

// Actuator Setup ActuatorModule(PUMP_PIN,FAN_PIN,LIGHT_PIN)
ActuatorModule actuator(PUMP_PIN, FAN_PIN, LIGHT_PIN);

void connectToWiFi()
{
  Serial.print("Connecting to WiFi");
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
}

void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client", AIO_USERNAME, AIO_KEY)) {
      Serial.println(" connected!");
      mqttClient.subscribe(MQTT_TOPIC);
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

// MQTT Callback Function 
void mqttCallback(char* topic, byte* payload, unsigned int length) { 
  StaticJsonDocument<100> doc; 
  DeserializationError error = deserializeJson(doc, payload, length); 
   
  if (error) return; 
 
  if (doc.containsKey("fan")) { 
    String state = doc["fan"]; 
    // digitalWrite(FAN_PIN, (state == "ON") ? HIGH : LOW); 
    actuator.setFan(state == "ON" ? true : false );
  } 
 
  if (doc.containsKey("pump")) { 
    String state = doc["pump"]; 
    // digitalWrite(PUMP_PIN, (state == "ON") ? HIGH : LOW); 
    actuator.setPump(state == "ON" ? true : false );

  } 
 
  if (doc.containsKey("light")) { 
    String state = doc["light"]; 
    // digitalWrite(LIGHT_PIN, (state == "ON") ? HIGH : LOW); 
    actuator.setLight(state == "ON" ? true : false );

  } 
}

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  sensor.begin();
  actuator.begin();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  for (int i = 0; i < 4; ++i)
  {
    sensor.addPlant(i, SOIL_PINS[i], PLANT_IDS[i]);
  }
  connectToMQTT();

}

void loop()
{
  sensor.sendAllToCloud(SERVER_URL, USER_ID);
  delay(60000); // 60s interval

  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();  // check for MQTT messages
  Serial.print("Started...");

}
