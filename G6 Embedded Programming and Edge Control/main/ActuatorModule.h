#ifndef ACTUATORMODULE_H
#define ACTUATORMODULE_H

#include <Arduino.h>

class Actuator {
  private:
    int pumpPin;
    int fanPin;
    int lightPin;

  public:
    Actuator(int pump, int fan, int light);
    void begin();
    void setPump(bool state);
    void setFan(bool state);
    void setLight(bool state);
};

#endif
