#ifndef RESTCLIENT_H
#define RESTCLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <utility> // for std::pair

class RESTClient {
public:
    RESTClient(const String &serverUrl, bool insecure = true);

    // Returns a vector of <plantId, soilPin> pairs from a zone
    std::vector<std::pair<String, int>> getPlantPinPairs(const String &zoneId);

    bool sendZoneSensorData(
        const String &zoneId,
        float temperature,
        float humidity,
        float light,
        float airQuality,
        const std::vector<std::pair<int, float>> &soilMoistureByPin,
        const String &userId = "",
        const String &timestamp = ""
    );

    // POST: Actuator log
    bool sendActuatorLog(
        const String &action,
        const String &actuatorId,
        const String &plantId,
        const String &trigger,
        const String &triggerBy = "SYSTEM",
        const String &timestamp = "",
        const String &action_name = ""
    );

private:
    String serverUrl;
    bool useInsecure;
};

#endif
