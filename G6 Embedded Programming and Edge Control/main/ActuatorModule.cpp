#include "ActuatorModule.h"

Actuator::Actuator(int pump, int fan, int light) {
  pumpPin = pump;
  fanPin = fan;
  lightPin = light;
}

void Actuator::begin() {
  pinMode(pumpPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
}

void Actuator::setPump(bool state) {
  digitalWrite(pumpPin, state ? HIGH : LOW);
}

void Actuator::setFan(bool state) {
  digitalWrite(fanPin, state ? HIGH : LOW);
}

void Actuator::setLight(bool state) {
  digitalWrite(lightPin, state ? HIGH : LOW);
}
