#include <CertStoreBearSSL.h>
#include <time.h>
#include <FS.h>

BearSSL::CertStore certStore;
unsigned int numCerts;

// heavy object, pre-allocate
BearSSL::WiFiClientSecure wifiClientSecure;

// Set time via NTP, as required for x.509 validation
boolean setClock() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // UTC

  Serial.print(F("Waiting for NTP time sync: "));
  time_t now = time(nullptr);
  unsigned int tries = 0;

  while (now < 8 * 3600 * 2) {
    if (tries >= 10 * 2) {
      Serial.println(F("Timeout"));
      return clockSet;
    }

    delay(500);
    Serial.print(F("."));
    now = time(nullptr);
    tries++;
  }

  tm timeinfo;
  gmtime_r(&now, &timeinfo);

  Serial.println();
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
  return clockSet = true;
}

void setupCert() {
  SPIFFS.begin();

  numCerts = certStore.initCertStore(SPIFFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.print(F("Number of CA certs read: "));
  Serial.println(numCerts);

  if (numCerts <= 0) {
    Serial.println(F("No certs found. Run certs-from-mozilla.py and upload the SPIFFS directory."));
  }

  wifiClientSecure.setCertStore(&certStore);
}
