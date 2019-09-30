#include <CertStoreBearSSL.h>
#include <time.h>
#include <FS.h>

class SPIFFSCertStoreFile : public BearSSL::CertStoreFile {
  public:
    SPIFFSCertStoreFile(const char* name) {
      _name = name;
    };
    virtual ~SPIFFSCertStoreFile() override {};

    virtual bool open(bool write = false) override {
      return _file = SPIFFS.open(_name, write ? "w" : "r");
    }
    virtual bool seek(size_t absolute_pos) override {
      return _file.seek(absolute_pos, SeekSet);
    }
    virtual ssize_t read(void* dest, size_t bytes) override {
      return _file.readBytes(static_cast<char*>(dest), bytes);
    }
    virtual ssize_t write(void* dest, size_t bytes) override {
      return _file.write(static_cast<uint8_t*>(dest), bytes);
    }
    virtual void close() override {
      _file.close();
    }

  private:
    File _file;
    const char* _name;
};

BearSSL::CertStore certStore;
SPIFFSCertStoreFile certs_idx("/certs.idx"); // generated
SPIFFSCertStoreFile certs_ar("/certs.ar"); // provided
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

  numCerts = certStore.initCertStore(&certs_idx, &certs_ar);
  Serial.print(F("Number of CA certs read: "));
  Serial.println(numCerts);

  if (numCerts <= 0) {
    Serial.println(F("No certs found. Run certs-from-mozilla.py and upload the SPIFFS directory."));
  }

  wifiClientSecure.setCertStore(&certStore);
}
