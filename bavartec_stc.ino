#include "types.h"

const double KELVIN = 273.15;

Config config;

void setup() {
  setupGPIO();

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  load();

  setupWiFi();
  setupServer();

  Serial.println(F("setup done"));

  schedule(false);
}

void loop() {
  loopGPIO();
  loopWiFi();
  loopServer();

  schedule(false);
}

void schedule(const boolean longer) {
  // seems to help with scheduling
  delay(longer ? 200 : 8);
}
