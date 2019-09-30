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

  if (isnan(config.adccal.slope)) {
    config.adccal.slope = 1.0;
  }

  if (isnan(config.adccal.offset)) {
    config.adccal.offset = 0.0;
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

  if (isnan(config.nightValue)) {
    config.nightValue = 17 + KELVIN;
  }

  if (isnan(config.slope)) {
    config.slope = 1.4;
  }

  if (config.weekly[0] == UCHAR_MAX) {
    for (size_t i = 0; i < sizeof(config.weekly); i++) {
      config.weekly[i] = false;
    }
  }

  if (config.nightly[0] == UCHAR_MAX) {
    for (size_t i = 0; i < sizeof(config.nightly); i++) {
      config.nightly[i] = false;
    }
  }

  if (config.mqtt.server[0] == UCHAR_MAX) {
    config.mqtt.server[0] = '\0';
    save(config.mqtt.server, sizeof(config.mqtt.server), "mqtt.bavartec.de");
  }

  if (config.mqtt.port == UINT_MAX) {
    config.mqtt.port = 8883;
  }

  if (config.mqtt.user[0] == UCHAR_MAX) {
    config.mqtt.user[0] = '\0';
  }

  if (config.mqtt.pass[0] == UCHAR_MAX) {
    config.mqtt.pass[0] = '\0';
  }
}

boolean reconfig(const String &key, const String &value) {
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
  } else if (key.equals("nightValue")) {
    config.nightValue = value.toDouble();
  } else if (key.equals("slope")) {
    config.slope = value.toDouble();
  } else if (key.equals("weekly")) {
    assert(value.length() == sizeof(config.weekly));

    for (size_t i = 0; i < sizeof(config.weekly); i++) {
      const char c = value.charAt(i);

      if (c == '0') {
        config.weekly[i] = false;
      } else if (c == '1') {
        config.weekly[i] = true;
      }
    }
  } else if (key.equals("nightly")) {
    assert(value.length() == sizeof(config.nightly));

    for (size_t i = 0; i < sizeof(config.nightly); i++) {
      const char c = value.charAt(i);

      if (c == '0') {
        config.nightly[i] = false;
      } else if (c == '1') {
        config.nightly[i] = true;
      }
    }
  } else {
    return false;
  }

  Serial.print(F("config."));
  Serial.print(key);
  Serial.print(F(" = "));
  Serial.println(value);
  return true;
}

void save() {
  EEPROM.begin(sizeof(config));
  EEPROM.put(0, config);
  EEPROM.end();
}
