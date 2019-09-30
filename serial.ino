char serialBuffer[128];
int serialPointer = 0;

void loopSerial() {
  while (Serial.available()) {
    const char c = Serial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      serialBuffer[serialPointer] = '\0';
      const String input = serialBuffer;
      serialTerm(input);
      serialPointer = 0;
      continue;
    }

    serialBuffer[serialPointer] = c;
    serialPointer++;
    serialPointer %= sizeof(serialBuffer);
  }
}

void serialTerm(const String &input) {
  const int space0 = input.indexOf(' ');

  if (space0 <= 0) {
    return;
  }

  Serial.print(F("Received \""));
  Serial.print(input);
  Serial.println(F("\""));

  const String cmd = input.substring(0, space0);

  if (cmd.equals("FLAG")) {
    const String flag = input.substring(space0 + 1);

    if (flag.equals("redpencil")) {
      redpencil = true;
    }
  } else if (cmd.equals("SENSOR")) {
    const String sensor = input.substring(space0 + 1);
    config.sensor = sensorByName(sensor.c_str());
    config.eich = eichSensor(config.sensor);
  } else if (cmd.equals("WIFI")) {
    const int space1 = input.indexOf(' ', space0 + 1);
    const String ssid = input.substring(space0 + 1, space1);
    const String pass = input.substring(space1 + 1);

    save(config.wifi.ssid, sizeof(config.wifi.ssid), ssid);
    save(config.wifi.pass, sizeof(config.wifi.pass), pass);
    resetWifi();
  }
}
