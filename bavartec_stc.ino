#include <assert.h>
#include <ESP8266httpUpdate.h>

#include "types.h"

constexpr double KELVIN = 273.15;

Config config;
boolean clockSet = false;

const String to_string(const double value, const int width, const char format[]) {
  char numstr[width];
  sprintf(numstr, format, value);
  return numstr;
}

int weekHour() {
  if (!clockSet) {
    return -1;
  }

  const time_t now = time(nullptr);
  const tm* timeinfo = localtime(&now);
  return timeinfo->tm_wday * 24 + timeinfo->tm_hour;
}

void setup() {
  setupGPIO();

  Serial.begin(115200);
  Serial.println();
  Serial.println();

  load();

  setupWiFi();
  setupServer();
  setupCert();
  setupOTA();
  setupMQTT();

  Serial.println(F("setup done"));

  schedule(false);
}

void loop() {
  loopGPIO();
  loopWiFi();
  loopServer();
  loopOTA();
  loopMQTT();
  loopSerial();

  schedule(false);
}

void schedule(const boolean longer) {
  // seems to help with scheduling
  delay(longer ? 200 : 8);
}
