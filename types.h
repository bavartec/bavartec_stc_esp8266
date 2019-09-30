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

struct Config {
  WifiConfig wifi;
};
