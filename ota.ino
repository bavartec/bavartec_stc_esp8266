#include <ESP8266httpUpdate.h>

const char* VERSION = "yyyy-mm-dd-HH-MM";
const char* VERSION_DATA = "yyyy-mm-dd-HH-MM";

void loopOTA() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  static unsigned long lastUpdate = 1E9;
  static unsigned long lastChecked = 1E9;
  const unsigned long time = millis();

  if (time - lastUpdate < 7 * 24 * 3600 * 1000 || time - lastChecked < 6 * 3600 * 1000) {
    return;
  }

  if (updateNow()) {
    lastUpdate = time;
  }

  lastChecked = time;
}

boolean updateNow() {
  if (!numCerts || !setClock()) {
    return false;
  }

  BearSSL::WiFiClientSecure wifiClient;
  wifiClient.setCertStore(&certStore);

  if (wifiClient.probeMaxFragmentLength("www.bavartec.de", 443, 1024)) {
    wifiClient.setBufferSizes(1024, 1024);
  }

  // order is important because data version is hardcoded into firmware
  Serial.println(F("Updating data..."));
  logUpdate(ESPhttpUpdate.updateSpiffs(wifiClient, "https://www.bavartec.de/php/ota.php?file=stc-data", VERSION_DATA));
  Serial.println(F("Updating firmware..."));
  logUpdate(ESPhttpUpdate.update(wifiClient, "https://www.bavartec.de/php/ota.php?file=stc", VERSION));

  return true;
}

void logUpdate(const HTTPUpdateResult result) {
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Serial.printf_P(PSTR("HTTP_UPDATE_FAILED Error (%d): %s\n"), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      // might restart before this is called
      Serial.println(F("HTTP_UPDATE_OK"));
      break;
  }
}
