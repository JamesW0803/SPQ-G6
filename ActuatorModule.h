#ifndef ACTUATORMODULE_H
#define ACTUATORMODULE_H

#include <Arduino.h>

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoJson.h>

class ActuatorModule {
  private:
    int pumpPin;
    int fanPin1;
    int fanPin2;
    int lightPin;
    Adafruit_MQTT_Publish* publishFeed;
    Adafruit_MQTT_Subscribe* subscribeFeed;

  public:
    ActuatorModule(int pump, int fan1, int fan2, int light, Adafruit_MQTT_Publish* publish, Adafruit_MQTT_Subscribe* subscribe);
    void begin();
    void setPump(bool state);
    void setFan(bool state);
    void callback(Adafruit_MQTT_Subscribe* subscription);
    // void setFan2(bool state);
    void setLight(bool state);
};

#endif
