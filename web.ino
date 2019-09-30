#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void setupServer() {
  server.on("/config/wifi", handleConfigWifi);

  server.begin();
}

void loopServer() {
  server.handleClient();
}

void flushServer() {
  server.client().flush();
  schedule(true);
}

void handleConfigWifi() {
  const String ssidArg = server.arg("ssid");
  const String passArg = server.arg("pass");

  config.wifi = WifiConfig(ssidArg, passArg);
  save();

  server.send(204);
  flushServer();

  resetWifi();
}
