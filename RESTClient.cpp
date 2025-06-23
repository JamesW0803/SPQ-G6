#include "RESTClient.h"

RESTClient::RESTClient(const String &serverUrl, bool insecure) {
    this->serverUrl = serverUrl;
    this->useInsecure = insecure;
}

std::vector<std::pair<String, int>> RESTClient::getPlantPinPairs(const String &zoneId) {
    std::vector<std::pair<String, int>> plantPairs;

    String endpoint = serverUrl + "/api/v1/zones/" + zoneId + "/plants";

    WiFiClientSecure client;
    if (useInsecure) {
        client.setInsecure();  // Skip TLS certificate validation
    }

    HTTPClient http;
    http.begin(client, endpoint);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response);
        if (error) {
            Serial.print("JSON parse error: ");
            Serial.println(error.c_str());
            http.end();
            return plantPairs; // Return empty vector
        }

        for (JsonObject plant : doc.as<JsonArray>()) {
            String plantId = plant["id"].as<String>();
            int soilPin = plant["soilPin"].as<int>();
            plantPairs.push_back(std::make_pair(plantId, soilPin));
        }

    } else {
        Serial.print("HTTP error code: ");
        Serial.println(httpResponseCode);
    }

    http.end();
    return plantPairs;
}

bool RESTClient::sendZoneSensorData(
    const String &zoneId,
    float temperature,
    float humidity,
    float light,
    float airQuality,
    const std::vector<std::pair<int, float>> &soilMoistureByPin,
    const String &userId,
    const String &timestamp
) {
    String endpoint = serverUrl + "/api/v1/sensor-data/";

    WiFiClientSecure client;
    if (useInsecure) {
        client.setInsecure();
    }

    HTTPClient http;
    http.begin(client, endpoint);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<1024> doc;

    doc["zoneId"] = zoneId;

    JsonObject zoneSensors = doc.createNestedObject("zoneSensors");
    zoneSensors["humidity"] = humidity;
    zoneSensors["temp"] = temperature;
    zoneSensors["light"] = light;
    zoneSensors["airQuality"] = airQuality;

    JsonArray soilArray = doc.createNestedArray("soilMoistureByPin");
    for (const auto &pair : soilMoistureByPin) {
        JsonObject entry = soilArray.createNestedObject();
        entry["pin"] = pair.first;
        entry["soilMoisture"] = pair.second;
    }

    if (userId != "") doc["userId"] = userId;
    if (timestamp != "") doc["timestamp"] = timestamp;

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
        Serial.print("Zone sensor data sent. HTTP code: ");
        Serial.println(httpResponseCode);
        http.end();
        return true;
    } else {
        Serial.print("Failed to send zone sensor data. Code: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

bool RESTClient::sendActuatorLog(
    const String &action,
    const String &actuatorId,
    const String &plantId,
    const String &trigger,
    const String &triggerBy,
    const String &timestamp,
    const String &action_name
) {
    String endpoint = serverUrl + "/api/v1//logs/action/" + action_name; // Adjust path if needed

    WiFiClientSecure client;
    if (useInsecure) {
        client.setInsecure();
    }

    HTTPClient http;
    http.begin(client, endpoint);
    http.addHeader("Content-Type", "application/json");

    // Prepare JSON payload
    StaticJsonDocument<256> doc;
    doc["action"] = action;
    doc["actuatorId"] = actuatorId;
    doc["plantId"] = plantId;
    doc["trigger"] = trigger;
    if (triggerBy != "") {
        doc["triggerBy"] = triggerBy;
    }
    if (timestamp != "") {
        doc["timestamp"] = timestamp;
    }

    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("POST success: ");
        Serial.println(response);
        http.end();
        return true;
    } else {
        Serial.print("POST failed, error: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}
