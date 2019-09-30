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

  if (cmd.equals("ADCCAL")) {
    const int space1 = input.indexOf(' ', space0 + 1);
    const double x1 = input.substring(space0 + 1, space1).toDouble();
    const double x2 = input.substring(space1 + 1).toDouble();

    const double r = 1 / (1 / 100.0 + 1 / 1e6);
    const double y1 = 3.3 * r / (r + inputReference(INPUT_TYPE::P10));
    const double y2 = 3.3 * r / (r + 1 / (1 / inputReference(INPUT_TYPE::P1) + 1 / inputReference(INPUT_TYPE::P5)));

    const double slope = (y2 - y1) / (x2 - x1);
    const double offset = y2 - slope * x2;
    config.adccal = AdcCal(slope, offset);
    save();
  } else if (cmd.equals("FLAG")) {
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
