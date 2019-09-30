enum class SENSOR : uint8_t {
  UNKNOWN = 0xFF,
  NTC1K = 0, NTC2K, NTC5K, NTC10K, NTC20K,
  PT100, PT500, PT1000,
  NI100, NI500, NI1000, NI1000TK5000,
  KTY1K, KTY2K,
};
const uint8_t lastSensor = static_cast<uint8_t>(SENSOR::KTY2K);

enum class SENSOR_TYPE : uint8_t {
  UNKNOWN = 0xFF,
  NTC = 0, PTC,
};

enum class INPUT_TYPE : uint8_t {
  UNKNOWN = 0xFF,
  P1 = 0, P5, P10, N2, N10, N20,
};
const uint8_t lastInputType = static_cast<uint8_t>(INPUT_TYPE::N20);

enum class OUTPUT_TYPE : uint8_t {
  UNKNOWN = 0xFF,
  NTC = 0, PTC,
};

void save(char key[], const int keySize, const String &value) {
  strlcpy(key, value.c_str(), keySize);
}

struct WifiConfig {
  char ssid[32];
  char pass[64];

  WifiConfig() {}

  WifiConfig(const String &ssid, const String &pass) {
    save(this->ssid, sizeof(this->ssid), ssid);
    save(this->pass, sizeof(this->pass), pass);
  }
};

struct Eich {
  double beta;
  double r0;
  double t0;

  Eich() {}

  Eich(const double beta, const double r0, const double t0) {
    this->beta = beta;
    this->r0 = r0;
    this->t0 = t0;
  }
};

struct Config {
  WifiConfig wifi;

  SENSOR sensor;
  Eich eich;
  unsigned char enabled; // boolean

  double noControlValue;
  double controlValue;
};
