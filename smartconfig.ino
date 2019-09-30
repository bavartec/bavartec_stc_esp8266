#include <smartconfig.h>

const __FlashStringHelper* scStatusName(const sc_status status) {
  switch (status) {
    case SC_STATUS_WAIT: return F("SC_STATUS_WAIT");
    case SC_STATUS_FIND_CHANNEL: return F("SC_STATUS_FIND_CHANNEL");
    case SC_STATUS_GETTING_SSID_PSWD: return F("SC_STATUS_GETTING_SSID_PSWD");
    case SC_STATUS_LINK: return F("SC_STATUS_LINK");
    case SC_STATUS_LINK_OVER: return F("SC_STATUS_LINK_OVER");
  }

  return F("SC_STATUS_UNKNOWN");
}

boolean smartConfigActive = false;
sc_status smartConfigStatus;
unsigned long smartConfigStarted;

boolean beginSmartConfig() {
  Serial.println(F("begin smart config"));
  resetWifi();
  WiFi.mode(WIFI_STA);

  smartconfig_set_type(SC_TYPE_ESPTOUCH);
  esptouch_set_timeout(105);
  smartconfig_start(reinterpret_cast<sc_callback_t>(&smartConfigCallback), 1);

  smartConfigActive = true;
  smartConfigStatus = SC_STATUS_WAIT;
  smartConfigStarted = millis();
}

boolean stopSmartConfig() {
  smartconfig_stop();
  smartConfigActive = false;
  WiFi.mode(WIFI_AP_STA);
  Serial.println(F("stop smart config"));
}

void smartConfigCallback(sc_status status, void* result) {
  Serial.println(scStatusName(status));
  smartConfigStatus = status;

  if (status == SC_STATUS_LINK) {
    station_config* sta_conf = reinterpret_cast<station_config*>(result);

    memcpy(config.wifi.ssid, sta_conf->ssid, 32);
    memcpy(config.wifi.pass, sta_conf->password, 64);

    wifi_station_set_config(sta_conf);
    wifi_station_disconnect();
    wifi_station_connect();
    save();
  } else if (status == SC_STATUS_LINK_OVER) {
    stopSmartConfig();
  }
}

void loopSmartConfig() {
  if (pressTime > 0) {
    if (smartConfigActive) {
      stopSmartConfig();
    } else {
      beginSmartConfig();
    }
  }

  if (!smartConfigActive) {
    return;
  }

  if (smartConfigStatus == SC_STATUS_WAIT) {
    statusLED = 0x200;
  } else {
    statusLED = 0x100;
  }

  if (millis() - smartConfigStarted > 120 * 1000) {
    Serial.println(F("smart config timeout"));
    stopSmartConfig();
  }
}
