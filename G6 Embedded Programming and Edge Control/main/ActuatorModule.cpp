#include "ActuatorModule.h"

ActuatorModule::ActuatorModule(int pump, int fan, int light) {
  pumpPin = pump;
  fanPin = fan;
  lightPin = light;
}

void ActuatorModule::begin() {
  pinMode(pumpPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
}

void ActuatorModule::setPump(bool state) {
  digitalWrite(pumpPin, state ? HIGH : LOW);
}

void ActuatorModule::setFan(bool state) {
  digitalWrite(fanPin, state ? HIGH : LOW);
}

void ActuatorModule::setLight(bool state) {
  digitalWrite(lightPin, state ? HIGH : LOW);
}
