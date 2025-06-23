#include "ActuatorModule.h"

ActuatorModule::ActuatorModule(
  int pump, 
  int fan1, 
  int fan2,
  int light,
  Adafruit_MQTT_Publish* publish,
  Adafruit_MQTT_Subscribe* subscribe) {
    pumpPin = pump;
    fanPin1 = fan1;
    fanPin2 = fan2;
    lightPin = light;
    publishFeed = publish;
    subscribeFeed = subscribe;
}

void ActuatorModule::begin() {
  Serial.println("Initializing actuators...");
  // pinMode(pumpPin, OUTPUT);
  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);
  pinMode(lightPin, OUTPUT);
  Serial.println("Actuators initialized.");
}

void ActuatorModule::callback(Adafruit_MQTT_Subscribe* subscription) {
    // Ensure the message came from the correct topic
    if (subscribeFeed && strcmp(subscription->topic, subscribeFeed->topic) == 0) {
        const char* command = (char*)subscribeFeed->lastread;

        // Parse JSON payload
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, command);

        if (error) {
            Serial.print("JSON parse failed: ");
            Serial.println(error.c_str());
            return;
        }

        // Read JSON fields
        const char* light = doc["light"];
        const char* fan = doc["fan"];

        Serial.print("Light: ");
        Serial.println(light);
        Serial.print("Fan: ");
        Serial.println(fan);

        // Control actuators based on command values
        digitalWrite(lightPin, strcmp(light, "ON") == 0 ? HIGH : LOW);
        digitalWrite(fanPin1, strcmp(fan, "ON") == 0 ? HIGH : LOW);
    }
}

void ActuatorModule::setPump(bool state) {
  digitalWrite(pumpPin, state ? HIGH : LOW);
  
}

void ActuatorModule::setFan(bool state) {
  digitalWrite(fanPin1, state ? HIGH : LOW);
  digitalWrite(fanPin2, state ? HIGH : LOW);

}

void ActuatorModule::setLight(bool state) {
  digitalWrite(lightPin, state ? HIGH : LOW);
}
