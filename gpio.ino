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

  const boolean active = config.enabled && config.sensor != SENSOR::UNKNOWN;

  setActive(active);

  if (!active) {
    return;
  }

  coreGPIO();
}

void coreGPIO() {
  sensorRead(typeInput(config.sensor), sensorReading, sensorVoltage, sensorResistance);

  sensorTemperature = toTemperature(config.sensor, config.eich, sensorResistance);
  controlTemperature = controlFunc(sensorTemperature);
  controlResistance = toResistance(config.sensor, config.eich, controlTemperature);

  control(controlResistance, controlFrequency1, controlFrequency2, controlFrequency3, config.sensor);

  pwm_set_duty(round(controlFrequency1 * PWM_PERIOD), 0);
  pwm_set_duty(round(controlFrequency2 * PWM_PERIOD), 1);
  pwm_set_duty(round(controlFrequency3 * PWM_PERIOD), 2);
  pwm_start();
}

void sensorRead(const INPUT_TYPE type, double &reading, double &voltage, double &resistance) {
  reading = analogRead(A0) / 1024.0;
  voltage = reading * 1.0; // best fit?
  resistance = inputReference(type) * voltage / (3.3 - voltage);
  resistance = 1 / (1 / resistance - 1 / 1e6); // pulldown, TODO: diode
}

double controlFunc(const double sensorValue) {
  return sensorValue + config.noControlValue - config.controlValue;
}
