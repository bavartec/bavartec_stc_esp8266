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

  if (config.enabled == UCHAR_MAX) {
    config.enabled = false;
  }

  if (isnan(config.noControlValue)) {
    config.noControlValue = 20 + KELVIN;
  }

  if (isnan(config.controlValue)) {
    config.controlValue = 20 + KELVIN;
  }
}

void reconfig(const String &key, const String &value) {
  if (key.equals("enabled")) {
    if (value.equals("true")) {
      config.enabled = true;
    } else if (value.equals("false")) {
      config.enabled = false;
    }
  } else if (key.equals("noControlValue")) {
    config.noControlValue = value.toDouble();
  } else if (key.equals("controlValue")) {
    config.controlValue = value.toDouble();
  }
}

void save() {
  EEPROM.begin(sizeof(config));
  EEPROM.put(0, config);
  EEPROM.end();
}
