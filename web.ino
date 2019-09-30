#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

void setupServer() {
  server.on("/config/input", handleConfigInput);
  server.on("/config/mqtt", handleConfigMQTT);
  server.on("/config/sensor", handleConfigSensor);
  server.on("/config/wifi", handleConfigWifi);
  server.on("/control", handleControl);
  server.on("/debug/listen", handleListen);
  server.on("/debug/query", handleQuery);
  server.on("/restart", handleRestart);
  server.on("/update", handleUpdate);

  server.on("/generate_204", handleGenerate204);
  server.begin();
}

void loopServer() {
  server.handleClient();
}

void flushServer() {
  server.client().flush();
  schedule(true);
}

void handleGenerate204() {
  server.send(204);
}

void handleConfigInput() {
  const String typeArg = server.arg("type");

  if (typeArg.length() == 0) {
    String text = "type=";
    text += inputTypeName(INPUT_TYPE::P10);

    server.send(200, "text/plain", text.c_str());
    return;
  }

  const INPUT_TYPE type = inputTypeByName(typeArg.c_str());

  const boolean relayChanged = setActive(true);

  double reading, voltage, resistance;
  sensorRead(type, reading, voltage, resistance);

  if (relayChanged) {
    setActive(false);
  }

  String text = "value=";
  text += to_string(resistance, 10, "%.2f");
  text += "&type=";
  text += inputTypeName(configInput(type, resistance));

  for (uint8_t i = 0; i <= lastSensor; i++) {
    const SENSOR sensor = static_cast<SENSOR>(i);
    const double temperature = toTemperature(sensor, eichSensor(sensor), resistance);
    text += "&";
    text += sensorName(sensor);
    text += "=";
    text += to_string(temperature, 7, "%.2f");
  }

  server.send(200, "text/plain", text.c_str());
}

void handleConfigMQTT() {
  const String serverArg = server.arg("server");
  const String portArg = server.arg("port");
  const String userArg = server.arg("user");
  const String passArg = server.arg("pass");

  config.mqtt = MQTTConfig(serverArg, portArg, userArg, passArg);
  save();

  server.send(204);
  flushServer();

  resetMQTT();
}

void handleConfigSensor() {
  const String sensorArg = server.arg("sensor");
  const String eichArg = server.arg("eich");

  config.sensor = sensorByName(sensorArg.c_str());
  config.eich = eichSensor(config.sensor);

  double acc = config.eich.beta;
  double count = 1;

  int index = 0;

  while (index < eichArg.length()) {
    const int delimiter0 = eichArg.indexOf(' ', index);
    assert(delimiter0 > 0);

    int delimiter1 = eichArg.indexOf(' ', delimiter0 + 1);

    if (delimiter1 < 0) {
      delimiter1 = eichArg.length();
    }

    const double t = eichArg.substring(index, delimiter0).toDouble();
    const double r = eichArg.substring(delimiter0 + 1, delimiter1).toDouble();

    double weight;
    const double beta = calcBeta(config.sensor, t, r, weight);

    if (!isnan(beta) && weight >= 1) {
      acc += beta * weight;
      count += weight;
    }

    index = delimiter1 + 1;
  }

  config.eich.beta = acc / count;
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
  text += "&version-data=";
  text += VERSION_DATA;
  text += "&wifi.ssid=";
  text += config.wifi.ssid;
  text += "&adccal.slope=";
  text += to_string(config.adccal.slope, 7, "%.4f");
  text += "&adccal.offset=";
  text += to_string(config.adccal.offset, 8, "%.4f");
  text += "&sensor=";
  text += sensorName(config.sensor);
  text += "&enabled=";
  text += config.enabled ? "true" : "false";
  text += "&noControlValue=";
  text += to_string(config.noControlValue, 7, "%.2f");
  text += "&controlValue=";
  text += to_string(config.controlValue, 7, "%.2f");
  text += "&nightValue=";
  text += to_string(config.nightValue, 7, "%.2f");
  text += "&slope=";
  text += to_string(config.slope, 5, "%.2f");
  text += "&mqtt.server=";
  text += config.mqtt.server;
  text += "&mqtt.port=";
  text += config.mqtt.port;
  text += "&mqtt.user=";
  text += config.mqtt.user;
  server.send(200, "text/plain", text.c_str());
}

void handleRestart() {
  server.send(204);
  flushServer();

  ESP.restart();
}

void handleUpdate() {
  updateSoon = true;
  server.send(204);
}
