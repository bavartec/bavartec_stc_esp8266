#include <DNSServer.h>
#include <ESP8266mDNS.h>

const __FlashStringHelper* statusName(const wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS: return F("WL_IDLE_STATUS");
    case WL_NO_SSID_AVAIL: return F("WL_NO_SSID_AVAIL");
    case WL_SCAN_COMPLETED: return F("WL_SCAN_COMPLETED");
    case WL_CONNECTED: return F("WL_CONNECTED");
    case WL_CONNECT_FAILED: return F("WL_CONNECT_FAILED");
    case WL_WRONG_PASSWORD: return F("WL_WRONG_PASSWORD");
    case WL_CONNECTION_LOST: return F("WL_CONNECTION_LOST");
    case WL_DISCONNECTED: return F("WL_DISCONNECTED");
  }

  return F("WL_UNKNOWN");
}

const IPAddress apIP(172, 217, 28, 206);
const IPAddress netMask(255, 255, 255, 0);

const char SSID[] = "Smart Thermo Control";
const char PASSWORD[] = "smart-thermo-control";
const char HOSTNAME[] = "smart-thermo-control";

int wifiRetryCount = 0;
unsigned long wifiRetryWait = 0;

DNSServer dns;

boolean wifiAsUsual() {
  return WiFi.status() == WL_CONNECTED && !smartConfigActive;
}

void resetWifi() {
  wifiRetryCount = 0;
  wifiRetryWait = 0;
  WiFi.disconnect();
  schedule(true);
}

void setupWiFi() {
  WiFi.persistent(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  // reset to healthy state
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_AP_STA);
  schedule(false);

  setupAP();
  setupMDNS();
}

boolean setupAP() {
  WiFi.softAPConfig(apIP, apIP, netMask);

  if (!WiFi.softAP(SSID, PASSWORD, 1, true)) {
    Serial.println(F("access point setup failed"));
    return false;
  }

  const IPAddress ip = WiFi.softAPIP();
  Serial.print(F("access point IP: "));
  Serial.println(ip);

  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", ip);
  return true;
}

boolean setupMDNS() {
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println(F("MDNS setup failed"));
    return false;
  }

  MDNS.addService("http", "tcp", 80);
  return true;
}

void loopWiFi() {
  if (smartConfigActive)  {
    return;
  }

  loopSTA();

  const wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    statusLED = ULONG_MAX;
  } else if (status == WL_DISCONNECTED) {
    statusLED = 0x200;
  } else {
    statusLED = 0x0;
  }

  dns.processNextRequest();

  if (WiFi.isConnected()) {
    // WiFi::softAPgetStationNum breaks pending connections to AP
    MDNS.update();
  }
}

void loopSTA() {
  static wl_status_t lastStatus = WL_IDLE_STATUS;
  static unsigned long lastConnect = 1E9;

  const wl_status_t status = WiFi.status();
  const unsigned long time = millis();

  if (lastStatus != status) {
    Serial.println(statusName(status));

    if (lastStatus == WL_DISCONNECTED) {
      lastConnect = time;
    }

    if (status == WL_CONNECTED) {
      const IPAddress ip = WiFi.localIP();
      Serial.print(F("local station IP: "));
      Serial.println(ip);

      wifiRetryCount = 0;
      wifiRetryWait = 0;
    } else if (status == WL_NO_SSID_AVAIL) {
      wifiRetryCount = 0;
      wifiRetryWait = 1 << 16;
    }
  }

  lastStatus = status;

  if (config.wifi.ssid[0] == '\0' || status == WL_CONNECTED || status == WL_DISCONNECTED) {
    return;
  }

  if (time - lastConnect < wifiRetryWait) {
    return;
  }

  Serial.print(F("Connecting to WiFi: "));
  Serial.println(config.wifi.ssid);

  WiFi.disconnect(true);
  schedule(true);

  WiFi.hostname(HOSTNAME);
  WiFi.begin(config.wifi.ssid, config.wifi.pass);
  schedule(false);

  // randomized truncated binary exponential backoff
  wifiRetryWait = 1 << (10 + min(wifiRetryCount, 5));
  wifiRetryWait += RANDOM_REG32 % wifiRetryWait;
  wifiRetryCount++;
}
