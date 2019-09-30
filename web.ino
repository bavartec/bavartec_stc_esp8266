#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void setupServer() {
  server.on("/config/sensor", handleConfigSensor);
  server.on("/config/wifi", handleConfigWifi);
  server.on("/control", handleControl);
  server.on("/debug/listen", handleListen);
  server.on("/debug/query", handleQuery);
  server.on("/restart", handleRestart);

  server.begin();
}

void loopServer() {
  server.handleClient();
}

void flushServer() {
  server.client().flush();
  schedule(true);
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

void handleListen() {
  String text = "sensorReading=";
  text += to_string(sensorReading, 7, "%.4f");
  text += "&sensorVoltage=";
  text += to_string(sensorVoltage, 6, "%.3f");
  text += "&sensorResistance=";
  text += to_string(sensorResistance, 10, "%.2f");
  text += "&sensorTemperature=";
  text += to_string(sensorTemperature, 7, "%.2f");
  text += "&controlTemperature=";
  text += to_string(controlTemperature, 7, "%.2f");
  text += "&controlResistance=";
  text += to_string(controlResistance, 10, "%.2f");
  text += "&controlFrequency1=";
  text += to_string(controlFrequency1, 7, "%.4f");
  text += "&controlFrequency2=";
  text += to_string(controlFrequency2, 7, "%.4f");
  text += "&controlFrequency3=";
  text += to_string(controlFrequency3, 7, "%.4f");
  server.send(200, "text/plain", text.c_str());
}

void handleQuery() {
  String text = "macSTA=";
  text += WiFi.macAddress().c_str();
  text += "&macAP=";
  text += WiFi.softAPmacAddress().c_str();
  text += "&version=";
  text += VERSION;
  text += "&wifi.ssid=";
  text += config.wifi.ssid;
  text += "&sensor=";
  text += sensorName(config.sensor);
  text += "&enabled=";
  text += config.enabled ? "true" : "false";
  text += "&noControlValue=";
  text += to_string(config.noControlValue, 7, "%.2f");
  text += "&controlValue=";
  text += to_string(config.controlValue, 7, "%.2f");
  server.send(200, "text/plain", text.c_str());
}

void handleRestart() {
  server.send(204);
  flushServer();

  ESP.restart();
}
