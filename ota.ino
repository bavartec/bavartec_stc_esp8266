#define ATOMIC_FS_UPDATE

#include <ESP8266httpUpdate.h>

// yyyy-mm-dd-HH-MM-SS
const char VERSION[] = "";
char VERSION_DATA[20];

boolean updateSoon = false;

void setupOTA() {
  File versionFile = SPIFFS.open("/VERSION", "r");

  if (versionFile) {
    String versionString = versionFile.readString();
    versionString.trim();
    strlcpy(VERSION_DATA, versionString.c_str(), sizeof(VERSION_DATA));
    versionFile.close();
  }
}

void loopOTA() {
  if (!wifiAsUsual()) {
    return;
  }

  static unsigned long lastUpdate = 1E9;
  static unsigned long lastChecked = 1E9;
  const unsigned long time = millis();

  if (!updateSoon && (time - lastUpdate < 7 * 24 * 3600 * 1000 || time - lastChecked < 6 * 3600 * 1000)) {
    return;
  }

  if (updateNow()) {
    lastUpdate = time;
  }

  lastChecked = time;
  updateSoon = false;
}

boolean updateNow() {
  if (!numCerts || !setClock()) {
    return false;
  }

  if (wifiClientSecure.connected()) {
    wifiClientSecure.stop();
  }

  if (wifiClientSecure.probeMaxFragmentLength("www.bavartec.de", 443, 1024)) {
    wifiClientSecure.setBufferSizes(1024, 1024);
  }

  Serial.print(F("Updating firmware: "));
  logUpdate(ESPhttpUpdate.update(wifiClientSecure, "https://www.bavartec.de/ota/stc", VERSION));

  Serial.print(F("Updating data: "));
  const boolean newFS = logUpdate(ESPhttpUpdate.updateFS(wifiClientSecure, "https://www.bavartec.de/ota/stc-data", VERSION_DATA));

  if (newFS) {
    setupOTA();
  }

  return true;
}

boolean logUpdate(const HTTPUpdateResult result) {
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Serial.printf_P(PSTR("HTTP_UPDATE_FAILED Error (%d): %s\n"), ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
      return false;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(F("HTTP_UPDATE_NO_UPDATES"));
      return false;

    case HTTP_UPDATE_OK:
      // might restart before this is called
      Serial.println(F("HTTP_UPDATE_OK"));
      return true;
  }

  return false;
}
