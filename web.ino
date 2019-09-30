#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void setupServer() {
  server.on("/", handleIndex);
  server.on("/config/sensor", handleConfigSensor);
  server.on("/config/wifi", handleConfigWifi);
  server.on("/control", handleControl);

  server.begin();
}

void loopServer() {
  server.handleClient();
}

void flushServer() {
  server.client().flush();
  schedule(true);
}

void handleIndex() {
  server.sendHeader("Location", "https://www.bavartec.de", true);
  server.send(302);
}

void handleConfigSensor() {
  const String sensorArg = server.arg("sensor");

  config.sensor = sensorByName(sensorArg.c_str());
  save();

  const String text = inputTypeName(typeInput(config.sensor));
  server.send(200, "text/plain", text.c_str());
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

void handleControl() {
  for (int i = 0; i < server.args(); i++) {
    const String key = server.argName(i);
    const String value = server.arg(i);

    if (value.length()) {
      reconfig(key, value);
    }
  }

  save();
  server.send(204);
}
