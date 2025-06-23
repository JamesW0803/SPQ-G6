#include <WiFi.h>
#include "SensorModule.h"
#include "ActuatorModule.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include "MqttModule.h"
#include "RESTClient.h"
#include "secrets.h"

// WIFI Configuration
const char *SSID = "Cynex@2.4GHz";
const char *PASSWORD = "cyber@cynex";
const String SERVER_URL = "https://test-server-owq2.onrender.com";
const String USER_ID = "5ZyZcYb6wpdlObgeJiaGZ0ydMbW2";

// // Adafruit IO MQTT config
// const char* MQTT_SERVER = "io.adafruit.com";
// const int MQTT_PORT = 1883;
// const char* MQTT_TOPIC = "SmartGrow/feeds/actuator-status";

// Adafruit IO MQTT server info
#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_USERNAME "SmartGrow"
#define MQTT_KEYS ""

// GPIO Definitions
const uint8_t SOIL_PINS[] = {34, 35, 36, 39};
const char *PLANT_IDS[] = {
    "plant_JU4Rj78DEHUM2lNYHzd3",
    "plant_CyhF06FW5a1KTmCvM0zf",
    "plant_hG7bPH7Np9WXtDe1zBVE",
    "plant_iKjnBJcBaGTx6LyCXUy2"
  };

// Actuator PIN
const int PUMP_PIN = 25;
const int FAN_PIN_1 = 27;
const int FAN_PIN_2 = 18;
const int LIGHT_PIN = 26;

// WiFi & MQTT Clients
WiFiClient wifiClient;
Adafruit_MQTT_Client mqtt(&wifiClient, MQTT_SERVER, MQTT_PORT, MQTT_USERNAME, MQTT_KEYS);
// PubSubClient mqttClient(wifiClient);

// Initialize RESTClient 
RESTClient restClient(SERVER_URL, true);

// MQTT Publish and Subscribe feeds
Adafruit_MQTT_Publish publishFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/group-1.actuator-status");
Adafruit_MQTT_Subscribe subscribeFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/feeds/group-1.actuator-status");

// Sensor Setup
SensorModule sensor(DHT_PIN, DHT_TYPE, (uint8_t *)SOIL_PINS, 4);

// Actuator Setup ActuatorModule(PUMP_PIN,FAN_PIN,LIGHT_PIN)
ActuatorModule actuator(PUMP_PIN, FAN_PIN_1, FAN_PIN_2, LIGHT_PIN, &publishFeed, &subscribeFeed);

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
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client123", AIO_USERNAME, AIO_KEY)) {
      Serial.println("MQTT connected!");
      mqttClient.subscribe(MQTT_TOPIC);
    } else {
      Serial.print(" failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

// // MQTT Callback Function 
// void mqttCallback(char* topic, byte* payload, unsigned int length) { 
//   Serial.println("Signal received");
//   StaticJsonDocument<100> doc; 
//   DeserializationError error = deserializeJson(doc, payload, length); 
   
//   if (error) {
//     Serial.println("Error receiving signal");
//     return;
//   } 
 
//   if (doc.containsKey("fan")) { 
//     String state = doc["fan"]; 
//     // digitalWrite(FAN_PIN, (state == "ON") ? HIGH : LOW); 
//     actuator.setFan(state == "ON" ? true : false );
//     if(state == "ON"){
//         Serial.println("FAN ON!");
//     }else{
//         Serial.println("FAN OFF!");
//     }
//   } 
 
//   if (doc.containsKey("pump")) { 
//     String state = doc["pump"]; 
//     // digitalWrite(PUMP_PIN, (state == "ON") ? HIGH : LOW); 
//     actuator.setPump(state == "ON" ? true : false );

//   } 
 
//   if (doc.containsKey("light")) { 
//     String state = doc["light"]; 
//     // digitalWrite(LIGHT_PIN, (state == "ON") ? HIGH : LOW); 
//     actuator.setLight(state == "ON" ? true : false );
//     if(state == "ON"){
//         Serial.println("Light ON!");
//     }else{
//         Serial.println("Light OFF!");
//     }

//   } 
// }

// edge control
void evaluateSensorsAndTrigger() {
  int lightValue = sensor.readLightLevel();
  int airQualityValue = sensor.readAirQuality();
  int tempValue = sensor.readTemperature();

  if (!sensor.fetchThresholdsFromAPI()) {
    Serial.println("Using default thresholds (0)");
  }

  bool lightBelow = sensor.checkAndTrigger("Light", lightValue, sensor.lightMax);
  bool airQualityBelow = sensor.checkAndTrigger("Air Quality", airQualityValue, sensor.airQualityMax);
  bool tempBelow= sensor.checkAndTrigger("Temperature", tempValue, sensor.tempMax);

  if (lightBelow) {
    Serial.println("Light out of range! Activate actuator.");
    actuator.setLight(true);
  }else{
    Serial.println("Light normal. Turning off grow light.");
    actuator.setLight(false);
  }

  if (airQualityBelow) {
    Serial.println("Air quality bad! Activate actuator.");
    actuator.setFan(true);
  }else{
    Serial.println("Air quality normal! Turning off actuator.");
    actuator.setFan(false);
  }

  if (tempBelow) {
      Serial.println("Temperature too low! Activate actuator.");

    }
}

void setup()
{
  Serial.begin(115200);
  connectToWiFi();
  mqtt.subscribe(&subscribeFeed);
  sensor.begin();
  actuator.begin();

  std::vector<std::pair<String, int>> plantMappings = restClient.getPlantPinPairs("zone4");
  
  if (plantMappings.size() == 0) {
    Serial.println("[ERROR] No plant mapping found!");
    return;
  }
  
  for (size_t i = 0; i < plantMappings.size(); ++i) {
    sensor.addPlant(i, plantMappings[i].second, plantMappings[i].first);
  }
  // mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  // mqttClient.setCallback(mqttCallback);

  // for (int i = 0; i < 4; ++i)
  // {
  //   sensor.addPlant(i, SOIL_PINS[i], PLANT_IDS[i]);
  // }

  // Fetch threshold per plant and map to soilPin
  if (sensor.fetchMoistureThresholdsForEachPlant(plantMappings)) {
    Serial.println("[OK] Moisture thresholds fetched for each plant.");
  } else {
    Serial.println("[WARN] Failed to fetch all moisture thresholds.");
  }
  // connectToMQTT();

}

void loop()
{
  MqttModule::connectToMqtt(mqtt);
  // Listen for MQTT messages
  // Adafruit_MQTT_Subscribe *subscription;
  // while ((subscription = mqtt.readSubscription(500))) {
  //   // Serial.println("[MQTT] Message received!");

  //   // actuator.callback(subscription); // Handle MQTT message
  //   if (subscription) {
  //     Serial.println("[MQTT] Message received!");
  //     actuator.callback(subscription);
  //   } else {
  //     Serial.println("[MQTT] No message");
  //   }
  // }

  Serial.println("[Loop] Checking for MQTT messages...");
  Adafruit_MQTT_Subscribe* subscription = mqtt.readSubscription(1000);

  if (subscription) {
    Serial.println("[Loop] Message received!");
    actuator.callback(subscription); // Handle message
  } else {
    Serial.println("[Loop] No message received.");
  }
  // Serial.println("Loop started...");
  // sensor.sendAllToCloud(SERVER_URL, USER_ID);
   evaluateSensorsAndTrigger();

  // Soil moisture watering logic
  // if (sensor.fetchThresholdsFromAPI()) {

  if (sensor.shouldWater()) {
    Serial.println("[PUMP] Watering needed → ON");
    digitalWrite(PUMP_PIN, HIGH);
    actuator.setPump(true);
  } else {
    Serial.println("[PUMP] Moisture OK → OFF");
    digitalWrite(PUMP_PIN, LOW);
    actuator.setPump(false);
  }

  // if (!mqttClient.connected()) {
  //   connectToMQTT();
  // }
  // mqttClient.loop();  // Handle MQTT
  delay(10000);       // Wait 10s before next loop
}