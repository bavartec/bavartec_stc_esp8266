#include <EEPROM.h>
#include <limits.h>

// factory state: all 0xFF bytes

void load() {
  EEPROM.begin(sizeof(config));
  EEPROM.get(0, config);
  EEPROM.end();

  if (config.wifi.ssid[0] == UCHAR_MAX) {
    config.wifi.ssid[0] = '\0';
  }

  if (config.wifi.pass[0] == UCHAR_MAX) {
    config.wifi.pass[0] = '\0';
  }
}

void save() {
  EEPROM.begin(sizeof(config));
  EEPROM.put(0, config);
  EEPROM.end();
}
