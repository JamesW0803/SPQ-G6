#include "ActuatorModule.h"

ActuatorModule::ActuatorModule(
  int pump, 
  int fan1, 
  int fan2,
  int light,
  Adafruit_MQTT_Publish* publish,
  Adafruit_MQTT_Publish* feedback,
  Adafruit_MQTT_Subscribe* subscribe
) {
    pumpPin = pump;
    fanPin1 = fan1;
    fanPin2 = fan2;
    lightPin = light;
    publishFeed = publish;
    feedbackFeed = feedback;
    subscribeFeed = subscribe;
}

void ActuatorModule::begin() {
  Serial.println("Initializing actuators...");
  pinMode(pumpPin, OUTPUT);
  pinMode(fanPin1, OUTPUT);
  pinMode(fanPin2, OUTPUT);
  pinMode(lightPin, OUTPUT);
  Serial.println("Actuators initialized.");
}

void ActuatorModule::sendFeedback(const String &action, const String &triggeredBy, const String &source, const String &zone, bool success) {
    StaticJsonDocument<256> doc;

    // SensorModule sensor;
    // String timestamp = sensor.getISO8601Time();

    doc["action"] = action;
    doc["triggeredBy"] = triggeredBy;
    doc["source"] = source;
    doc["zone"] = zone;
    // doc["timestamp"] = timestamp; // uses SensorModule's time
    doc["result"] = success ? "success" : "fail";

    char payload[256];
    serializeJson(doc, payload);

    if (feedbackFeed) {
        if (!feedbackFeed->publish(payload)) {
            Serial.println("Failed to publish actuator feedback");
        } else {
            Serial.println("Actuator feedback published");
        }
    }
}

void ActuatorModule::callback(Adafruit_MQTT_Subscribe* subscription) {
  Serial.println("ActuatorModule callback triggered");
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

        if (light != nullptr) {
          digitalWrite(lightPin, strcmp(light, "ON") == 0 ? HIGH : LOW);
        }

        // Control fan only if present
        if (fan != nullptr) {
          digitalWrite(fanPin1, strcmp(fan, "ON") == 0 ? HIGH : LOW);
          digitalWrite(fanPin2, strcmp(fan, "ON") == 0 ? HIGH : LOW);
        }


        // //feedback light
        // sendFeedback(
        //     String("light ") + (strcmp(light, "ON") == 0 ? "ON" : "OFF"),
        //     "user",
        //     "manual",
        //     "zone1",
        //     true
        // );
        // //feedback fan
        // sendFeedback(
        //     String("fan ") + (strcmp(fan, "ON") == 0 ? "ON" : "OFF"),
        //     "user",
        //     "manual",
        //     "zone1",
        //     true
        // );
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
