#define PWM_MAX_CHANNELS 3
#define PWM_USE_NMI 1
#define PWM_MIRROR_DUTY 0

#include "pwm.h"

constexpr uint32_t PWM_PERIOD = 5000; // * 200 ns

//{PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4, 4},
//{PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5, 5},

uint32_t io_info[PWM_MAX_CHANNELS][3] = {
  {PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13, 13},
  {PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12, 12},
  {PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14, 14},
};

uint32_t pwm_duty_init[PWM_MAX_CHANNELS] = {0, 0, 0};

constexpr uint8_t PIN_BUTTON = 4;
constexpr uint8_t PIN_RELAY = 5;
constexpr uint8_t PIN_CONTROL_1 = 13;
constexpr uint8_t PIN_CONTROL_2 = 12;
constexpr uint8_t PIN_CONTROL_3 = 14;
constexpr uint8_t PIN_STATUS_LED = 16;

unsigned long pressTime = 0;
unsigned long statusLED = 0x0;

double sensorReading;
double sensorVoltage;
double sensorResistance;
double sensorTemperature;
double controlTemperature;
double controlResistance;
double controlFrequency1;
double controlFrequency2;
double controlFrequency3;

// factory testing
boolean redpencil = false;

boolean checkSoon = false;

void setupGPIO() {
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_CONTROL_1, OUTPUT);
  pinMode(PIN_CONTROL_2, OUTPUT);
  pinMode(PIN_CONTROL_3, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_STATUS_LED, LOW);

  pwm_init(PWM_PERIOD, pwm_duty_init, PWM_MAX_CHANNELS, io_info);
}

void checkButton() {
  static boolean wasPressed = false;
  static unsigned long pressedSince;

  const boolean pressed = digitalRead(PIN_BUTTON);

  if (wasPressed == pressed) {
    pressTime = 0;
    return;
  }

  wasPressed = pressed;

  if (pressed) {
    pressedSince = millis();
    pressTime = 0;
  } else {
    pressTime = millis() - pressedSince;
  }
}

boolean setActive(const boolean active) {
  static boolean wasActive = false;

  if (wasActive == active) {
    return false;
  }

  digitalWrite(PIN_RELAY, active ? HIGH : LOW);
  wasActive = active;

  // max relay switch time
  delay(6);

  if (active) {
    // typical ADC charge time
    schedule(false);
  }

  return true;
}

void loopGPIO() {
  const unsigned long time = millis();

  checkButton();
  digitalWrite(PIN_STATUS_LED, time & statusLED ? HIGH : LOW);

  static unsigned long lastChecked = 1E9;
  static boolean lastGood = true;

  if (checkSoon) {
    lastChecked = time;
    lastGood = true;
    checkSoon = false;
  }

  const boolean probablyGood = lastGood || time - lastChecked >= 3600 * 1000;
  const boolean active = ((config.enabled && probablyGood) || redpencil) && config.sensor != SENSOR::UNKNOWN;

  setActive(active);

  if (!active) {
    return;
  }

  const boolean good = coreGPIO();

  if (good) {
    lastChecked = time;
    lastGood = true;
  } else if (!lastGood) {
    lastChecked = time;
  } else if (time - lastChecked >= 5 * 1000) {
    lastChecked = time;
    lastGood = false;
  }

  if (redpencil) {
    Serial.print(F("sensorReading: "));
    Serial.printf_P(PSTR("%.4f"), sensorReading);
    Serial.println();

    Serial.print(F("sensorResistance: "));
    Serial.printf_P(PSTR("%.2f"), sensorResistance);
    Serial.println();
  }
}

boolean coreGPIO() {
  const boolean sensorGood = sensorRead(typeInput(config.sensor), sensorReading, sensorVoltage, sensorResistance);

  sensorTemperature = toTemperature(config.sensor, config.eich, sensorResistance);
  controlTemperature = controlFunc(sensorTemperature);
  controlResistance = toResistance(config.sensor, config.eich, controlTemperature);

  const CONTROL_STATUS controlStatus = control(controlResistance, controlFrequency1, controlFrequency2, controlFrequency3, config.sensor);
  boolean controlGood = true;

  if (controlStatus != CONTROL_STATUS::GOOD) {
    const double high = toResistance(config.sensor, config.eich, max(sensorTemperature, controlTemperature));
    const double low = toResistance(config.sensor, config.eich, min(sensorTemperature, controlTemperature));

    double freq1, freq2, freq3;
    const boolean controlHigh = control(high, freq1, freq2, freq3, config.sensor) == CONTROL_STATUS::IS_HIGH;
    const boolean controlLow = control(low, freq1, freq2, freq3, config.sensor) == CONTROL_STATUS::IS_LOW;

    if (controlHigh || controlLow) {
      controlGood = false;
    }
  }

  pwm_set_duty(round(controlFrequency1 * PWM_PERIOD), 0);
  pwm_set_duty(round(controlFrequency2 * PWM_PERIOD), 1);
  pwm_set_duty(round(controlFrequency3 * PWM_PERIOD), 2);
  pwm_start();

  return sensorGood && controlGood;
}

double adcRead() {
  uint16_t acc = 0;

  // oversampling

  for (size_t i = 0; i < 16; i++) {
    acc += analogRead(A0);
  }

  // decimation

  return round(acc / 4.0 + 0.5) / 4096.0;
}

boolean sensorRead(const INPUT_TYPE type, double &reading, double &voltage, double &resistance) {
  reading = adcRead();
  voltage = reading * config.adccal.slope + config.adccal.offset; // linear regression
  resistance = inputReference(type) * voltage / (3.3 - voltage); // voltage divider
  resistance = 1 / (1 / resistance - 1 / 1e6); // pulldown resistor
  return reading < 1.0;
}

double controlFunc(const double sensorValue) {
  const int time = weekHour();

  const double noControlValue = config.nightly[time] && time >= 0 ? config.nightValue : config.noControlValue;
  const double controlValue = config.weekly[time] || time < 0 ? config.controlValue : config.nightValue;

  return sensorValue + (noControlValue - controlValue) * (1 / config.slope + 1);
}
