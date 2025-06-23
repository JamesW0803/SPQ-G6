#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Arduino.h>
#include <time.h>
#include <ArduinoJson.h>

// Sensor Pin Configuration
#define DHT_PIN 16
#define DHT_TYPE DHT11
#define MQ2_PIN 32
#define LDR_PIN 33

class SensorModule
{
public:
    SensorModule(uint8_t dhtPin, uint8_t dhtType, uint8_t *soilPins, int numPlants);
    float lightMin = 0;
    float lightMax = 0;
    float airQualityMin = 0;
    float airQualityMax = 0;
    float tempMin  = 0;
    float tempMax  = 0;
    float soilMin  = 0;
    float soilMax  = 0;
    void begin();
    void addPlant(int plantIndex, int soilPin, const String &plantId);
    void sendAllToCloud(const String &serverURL, const String &userId);
    bool fetchThresholdsFromAPI();
    bool checkAndTrigger(const String& sensorName, int sensorValue, float maxVal);
    float readSoilMoisture(int pin);
    float readTemperature();
    float readHumidity();
    float readAirQuality();
    float readLightLevel();
    bool shouldWaterPlants();

private:
    struct Plant
    {
        int soilPin;
        String plantId;
    };

    static const int MAX_PLANTS = 4;
    Plant plants[MAX_PLANTS];

    DHT dht;
    uint8_t *_soilPins;
    int _numPlants;

    // float readSoilMoisture(int pin);
    // float readTemperature();
    // float readHumidity();
    // float readAirQuality();
    // float readLightLevel();
    // bool shouldWaterPlants();
    String getISO8601Time();
};

#endif