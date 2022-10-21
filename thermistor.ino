const SENSOR sensorByName(const char* name) {
  if (strcmp_P(name, PSTR("NTC1K")) == 0) {
    return SENSOR::NTC1K;
  } else if (strcmp_P(name, PSTR("NTC2K")) == 0) {
    return SENSOR::NTC2K;
  } else if (strcmp_P(name, PSTR("NTC5K")) == 0) {
    return SENSOR::NTC5K;
  } else if (strcmp_P(name, PSTR("NTC10K")) == 0) {
    return SENSOR::NTC10K;
  } else if (strcmp_P(name, PSTR("NTC20K")) == 0) {
    return SENSOR::NTC20K;
  } else if (strcmp_P(name, PSTR("PT100")) == 0) {
    return SENSOR::PT100;
  } else if (strcmp_P(name, PSTR("PT500")) == 0) {
    return SENSOR::PT500;
  } else if (strcmp_P(name, PSTR("PT1000")) == 0) {
    return SENSOR::PT1000;
  } else if (strcmp_P(name, PSTR("NI100")) == 0) {
    return SENSOR::NI100;
  } else if (strcmp_P(name, PSTR("NI500")) == 0) {
    return SENSOR::NI500;
  } else if (strcmp_P(name, PSTR("NI1000")) == 0) {
    return SENSOR::NI1000;
  } else if (strcmp_P(name, PSTR("NI1000TK5000")) == 0) {
    return SENSOR::NI1000TK5000;
  } else if (strcmp_P(name, PSTR("KTY1K")) == 0) {
    return SENSOR::KTY1K;
  } else if (strcmp_P(name, PSTR("KTY2K")) == 0) {
    return SENSOR::KTY2K;
  }

  return SENSOR::UNKNOWN;
}

const __FlashStringHelper* sensorName(const SENSOR sensor) {
  switch (sensor) {
    case SENSOR::NTC1K: return F("NTC1K");
    case SENSOR::NTC2K: return F("NTC2K");
    case SENSOR::NTC5K: return F("NTC5K");
    case SENSOR::NTC10K: return F("NTC10K");
    case SENSOR::NTC20K: return F("NTC20K");
    case SENSOR::PT100: return F("PT100");
    case SENSOR::PT500: return F("PT500");
    case SENSOR::PT1000: return F("PT1000");
    case SENSOR::NI100: return F("NI100");
    case SENSOR::NI500: return F("NI500");
    case SENSOR::NI1000: return F("NI1000");
    case SENSOR::NI1000TK5000: return F("NI1000TK5000");
    case SENSOR::KTY1K: return F("KTY1K");
    case SENSOR::KTY2K: return F("KTY2K");
  }

  return F("UNKNOWN");
}

const INPUT_TYPE inputTypeByName(const char* name) {
  if (strcmp_P(name, PSTR("P1")) == 0) {
    return INPUT_TYPE::P1;
  } else if (strcmp_P(name, PSTR("P5")) == 0) {
    return INPUT_TYPE::P5;
  } else if (strcmp_P(name, PSTR("P10")) == 0) {
    return INPUT_TYPE::P10;
  } else if (strcmp_P(name, PSTR("N2")) == 0) {
    return INPUT_TYPE::N2;
  } else if (strcmp_P(name, PSTR("N10")) == 0) {
    return INPUT_TYPE::N10;
  } else if (strcmp_P(name, PSTR("N20")) == 0) {
    return INPUT_TYPE::N20;
  }

  return INPUT_TYPE::UNKNOWN;
}

const __FlashStringHelper* inputTypeName(const INPUT_TYPE type) {
  switch (type) {
    case INPUT_TYPE::P1: return F("P1");
    case INPUT_TYPE::P5: return F("P5");
    case INPUT_TYPE::P10: return F("P10");
    case INPUT_TYPE::N2: return F("N2");
    case INPUT_TYPE::N10: return F("N10");
    case INPUT_TYPE::N20: return F("N20");
  }

  return F("UNKNOWN");
}

const INPUT_TYPE configInput(const INPUT_TYPE type, const double r) {
  const double voltage = 0.99 * config.adccal.slope + config.adccal.offset; // linear regression
  const uint8_t typeIndex = static_cast<uint8_t>(type);

  if (typeIndex > 0) {
    const INPUT_TYPE typeLower = static_cast<INPUT_TYPE>(typeIndex - 1);
    double low = inputReference(typeLower) * voltage / (3.3 - voltage); // voltage divider
    low = 1 / (1 / low - 1 / 1e6); // pulldown resistor

    if (r <= low) {
      return typeLower;
    }
  }

  if (typeIndex < lastInputType) {
    const INPUT_TYPE typeHigher = static_cast<INPUT_TYPE>(typeIndex + 1);
    double high = inputReference(type) * voltage / (3.3 - voltage); // voltage divider
    high = 1 / (1 / high - 1 / 1e6); // pulldown resistor

    if (r > high) {
      return typeHigher;
    }
  }

  return type;
}

const SENSOR_TYPE typeSensor(const SENSOR sensor) {
  switch (sensor) {
    case SENSOR::NTC1K:
    case SENSOR::NTC2K:
    case SENSOR::NTC5K:
    case SENSOR::NTC10K:
    case SENSOR::NTC20K:
      return SENSOR_TYPE::NTC;

    case SENSOR::PT100:
    case SENSOR::PT500:
    case SENSOR::PT1000:
    case SENSOR::NI100:
    case SENSOR::NI500:
    case SENSOR::NI1000:
    case SENSOR::NI1000TK5000:
    case SENSOR::KTY1K:
    case SENSOR::KTY2K:
      return SENSOR_TYPE::PTC;
  }

  return SENSOR_TYPE::UNKNOWN;
}

const INPUT_TYPE typeInput(const SENSOR sensor) {
  switch (sensor) {
    case SENSOR::NTC1K:
    case SENSOR::NTC2K:
    case SENSOR::KTY2K:
      return INPUT_TYPE::N2;

    case SENSOR::NTC5K:
    case SENSOR::NTC10K:
      return INPUT_TYPE::N10;

    case SENSOR::NTC20K:
      return INPUT_TYPE::N20;

    case SENSOR::PT100:
    case SENSOR::NI100:
      return INPUT_TYPE::P1;

    case SENSOR::PT500:
    case SENSOR::NI500:
      return INPUT_TYPE::P5;

    case SENSOR::PT1000:
    case SENSOR::NI1000:
    case SENSOR::NI1000TK5000:
    case SENSOR::KTY1K:
      return INPUT_TYPE::P10;
  }

  return INPUT_TYPE::UNKNOWN;
}

const double inputReference(const INPUT_TYPE type) {
  switch (type) {
    case INPUT_TYPE::P1: return 330;
    case INPUT_TYPE::P5: return 1500;
    case INPUT_TYPE::P10: return 3300;
    case INPUT_TYPE::N2: return 22e3;
    case INPUT_TYPE::N10: return 150e3;
    case INPUT_TYPE::N20: return 470e3;
  }

  return nan("");
}

const OUTPUT_TYPE typeOutput(const SENSOR sensor) {
  switch (sensor) {
    case SENSOR::NTC1K:
    case SENSOR::NTC2K:
    case SENSOR::NTC5K:
    case SENSOR::NTC10K:
    case SENSOR::NTC20K:
    case SENSOR::KTY2K:
      return OUTPUT_TYPE::NTC;

    case SENSOR::PT100:
    case SENSOR::PT500:
    case SENSOR::PT1000:
    case SENSOR::NI100:
    case SENSOR::NI500:
    case SENSOR::NI1000:
    case SENSOR::NI1000TK5000:
    case SENSOR::KTY1K:
      return OUTPUT_TYPE::PTC;
  }

  return OUTPUT_TYPE::UNKNOWN;
}

const Eich eichSensor(const SENSOR sensor) {
  double beta;
  double r0;
  double t0;

  switch (sensor) {
    case SENSOR::NTC1K:
      beta = 3536;
      break;
    case SENSOR::NTC2K:
      beta = 3378;
      break;
    case SENSOR::NTC5K:
      beta = 3855;
      break;
    case SENSOR::NTC10K:
      beta = 3855;
      break;
    case SENSOR::NTC20K:
      beta = 4090;
      break;

    case SENSOR::PT100:
      beta = 0.39;
      break;
    case SENSOR::PT500:
      beta = 1.95;
      break;
    case SENSOR::PT1000:
      beta = 3.9;
      break;

    case SENSOR::NI100:
      beta = 0.556;
      break;
    case SENSOR::NI500:
      beta = 2.78;
      break;
    case SENSOR::NI1000:
      beta = 5.56;
      break;
    case SENSOR::NI1000TK5000:
      beta = 4.483;
      break;

    case SENSOR::KTY1K:
      beta = 7.12;
      break;
    case SENSOR::KTY2K:
      beta = 13.68;
      break;
  }

  switch (sensor) {
    case SENSOR::NTC1K:
      r0 = 1000;
      break;
    case SENSOR::NTC2K:
      r0 = 2000;
      break;
    case SENSOR::NTC5K:
      r0 = 5000;
      break;
    case SENSOR::NTC10K:
      r0 = 10000;
      break;
    case SENSOR::NTC20K:
      r0 = 20000;
      break;

    case SENSOR::PT100:
    case SENSOR::NI100:
      r0 = 100;
      break;
    case SENSOR::PT500:
    case SENSOR::NI500:
      r0 = 500;
      break;
    case SENSOR::PT1000:
    case SENSOR::NI1000:
    case SENSOR::NI1000TK5000:
      r0 = 1000;
      break;

    case SENSOR::KTY1K:
      r0 = 1000;
      break;
    case SENSOR::KTY2K:
      r0 = 2000;
      break;
  }

  switch (sensor) {
    case SENSOR::NTC1K:
    case SENSOR::NTC2K:
    case SENSOR::NTC5K:
    case SENSOR::NTC10K:
    case SENSOR::NTC20K:
      t0 = 25 + KELVIN;
      break;

    case SENSOR::PT100:
    case SENSOR::PT500:
    case SENSOR::PT1000:
    case SENSOR::NI100:
    case SENSOR::NI500:
    case SENSOR::NI1000:
    case SENSOR::NI1000TK5000:
      t0 = KELVIN;
      break;

    case SENSOR::KTY1K:
    case SENSOR::KTY2K:
      t0 = 25 + KELVIN;
      break;
  }

  return Eich(beta, r0, t0);
}

const Linearizer linearizeSensor(const SENSOR sensor) {
  double rp = INFINITY;
  double rs = 0;

  return Linearizer(rp, rs);
}

inline double linearize(const Linearizer &linear, const double rt) {
  return 1 / (1 / rt + 1 / linear.rp) + linear.rs;
}

inline double delinearize(const Linearizer &linear, const double r) {
  return 1 / (1 / (r - linear.rs) - 1 / linear.rp);
}

const double calcBeta(const SENSOR sensor, const double t, const double r, double &weight) {
  const Eich eich = eichSensor(sensor);
  const Linearizer linear = linearizeSensor(sensor);

  const double rt = delinearize(linear, r);

  switch (typeSensor(sensor)) {
    case SENSOR_TYPE::NTC:
      weight = rt / eich.t0;
      return log(rt / eich.r0) / (1 / t - 1 / eich.t0);
    case SENSOR_TYPE::PTC:
      weight = fabs(t - eich.t0);
      return (rt - eich.r0) / (t - eich.t0);
  }

  weight = 0;
  return nan("");
}

const double toTemperature(const SENSOR sensor, const Eich &eich, const double r) {
  const Linearizer linear = linearizeSensor(sensor);
  const double rt = delinearize(linear, r);

  switch (typeSensor(sensor)) {
    case SENSOR_TYPE::NTC:
      return eich.t0 * eich.beta / (eich.beta + eich.t0 * log(rt / eich.r0));
    case SENSOR_TYPE::PTC:
      return (rt - eich.r0) / eich.beta + eich.t0;
  }

  return nan("");
}

const double toResistance(const SENSOR sensor, const Eich &eich, const double t) {
  double rt = nan("");

  switch (typeSensor(sensor)) {
    case SENSOR_TYPE::NTC:
      rt = eich.r0 * exp(eich.beta * (1 / t - 1 / eich.t0));
      break;
    case SENSOR_TYPE::PTC:
      rt = (t - eich.t0) * eich.beta + eich.r0;
      break;
  }

  const Linearizer linear = linearizeSensor(sensor);
  return linearize(linear, rt);
}
