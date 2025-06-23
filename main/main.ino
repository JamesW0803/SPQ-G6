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

// Initialize RESTClient 
RESTClient restClient(SERVER_URL, true);

// MQTT Publish and Subscribe feeds
Adafruit_MQTT_Publish publishFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/group-1.actuator-status");
Adafruit_MQTT_Publish feedbackFeed = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/group-1.actuator-feedback");
Adafruit_MQTT_Subscribe subscribeFeed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/feeds/group-1.actuator-status");

// Sensor Setup
SensorModule sensor(DHT_PIN, DHT_TYPE, (uint8_t *)SOIL_PINS, 4);

// Actuator Setup ActuatorModule(PUMP_PIN,FAN_PIN,LIGHT_PIN)
ActuatorModule actuator(PUMP_PIN, FAN_PIN_1, FAN_PIN_2, LIGHT_PIN, &publishFeed, &feedbackFeed, &subscribeFeed);

std::vector<PlantData> plants;

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

  Serial.println("Relay ON (Pump OFF with NC)");
  digitalWrite(PUMP_PIN, HIGH);
  delay(5000); // wait 5 sec

  Serial.println("Relay OFF (Pump ON with NC)");
  digitalWrite(PUMP_PIN, LOW);
  delay(5000);

  plants = restClient.getPlantsByZone("zone4");

  for (const auto& p : plants) {
      Serial.println("Plant ID: " + p.plantId);
      Serial.print("Moisture pin: "); Serial.println(p.moisturePin);
      Serial.print("Moisture Threshold: "); Serial.print(p.min_moisture); Serial.print(" - "); Serial.println(p.max_moisture);
      Serial.print("Temperature Threshold: "); Serial.print(p.min_temperature); Serial.print(" - "); Serial.println(p.max_temperature);
      Serial.print("Light Threshold: "); Serial.print(p.min_light); Serial.print(" - "); Serial.println(p.max_light);
      Serial.print("Air Quality Threshold: "); Serial.print(p.min_airQuality); Serial.print(" - "); Serial.println(p.max_airQuality);
      Serial.println();
  }
}

void loop()
{
  // if (!mqtt.connected()) {
  //   MqttModule::connectToMqtt(mqtt);
  // }  

  // Check for any incoming messages
  // Adafruit_MQTT_Subscribe *subscription;
  // while ((subscription = mqtt.readSubscription(5000))) {
  //   if (subscription == &subscribeFeed) {
  //     String jsonStr = (char*)subscribeFeed.lastread;
  //     Serial.println("Received JSON: " + jsonStr);
  //     actuator.callback(subscription); // Handle MQTT message
  //   }
  // }

  // sensor.sendAllToCloud(SERVER_URL, USER_ID);
  //  evaluateSensorsAndTrigger();

  if (sensor.shouldWater(plants)) {
    Serial.println("[PUMP] Watering needed → ON");
    digitalWrite(PUMP_PIN, HIGH);
    actuator.setPump(true);
  } else {
    Serial.println("[PUMP] Moisture OK → OFF");
    digitalWrite(PUMP_PIN, LOW);
    actuator.setPump(false);
  }

  delay(1000);       // Wait 10s before next loop
}