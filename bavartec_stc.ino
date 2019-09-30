#include "types.h"

Config config;

void setup() {
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
  loopWiFi();
  loopServer();

  schedule(false);
}

void schedule(const boolean longer) {
  // seems to help with scheduling
  delay(longer ? 200 : 8);
}
