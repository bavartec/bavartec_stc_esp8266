#define PWM_MAX_CHANNELS 3
#define PWM_USE_NMI 1
#define PWM_MIRROR_DUTY 0

#include "pwm.h"

const uint32_t PWM_PERIOD = 5000; // * 200 ns

//{PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4, 4},
//{PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5, 5},

uint32_t io_info[PWM_MAX_CHANNELS][3] = {
  {PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13, 13},
  {PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12, 12},
  {PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14, 14},
};

uint32_t pwm_duty_init[PWM_MAX_CHANNELS] = {0, 0, 0};

const uint8_t PIN_RELAY = 5;
const uint8_t PIN_SENSOR_VCC = 4;
const uint8_t PIN_CONTROL_1 = 13;
const uint8_t PIN_CONTROL_2 = 12;
const uint8_t PIN_CONTROL_3 = 14;

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
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_SENSOR_VCC, OUTPUT);
  pinMode(PIN_CONTROL_1, OUTPUT);
  pinMode(PIN_CONTROL_2, OUTPUT);
  pinMode(PIN_CONTROL_3, OUTPUT);

  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_SENSOR_VCC, LOW);

  pwm_init(PWM_PERIOD, pwm_duty_init, PWM_MAX_CHANNELS, io_info);
}

boolean setActive(const boolean active) {
  static boolean lastActive = false;

  if (lastActive && !active) {
    digitalWrite(PIN_SENSOR_VCC, LOW);
    digitalWrite(PIN_RELAY, LOW);
    // max relay switch time
    delay(6);
    lastActive = false;
    return true;
  } else if (!lastActive && active) {
    digitalWrite(PIN_RELAY, HIGH);
    // max relay switch time
    delay(6);
    digitalWrite(PIN_SENSOR_VCC, HIGH);
    lastActive = true;
    return true;
  }

  return false;
}

void loopGPIO() {
  const boolean active = config.enabled && config.sensor != SENSOR::UNKNOWN;
  setActive(active);

  if (!active) {
    return;
  }

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
  reading = static_cast<double>(analogRead(A0)) / PWMRANGE;
  voltage = reading * 1.0; // best fit?
  resistance = inputReference(type) * voltage / (3.3 - voltage);
  resistance = 1 / (1 / resistance - 1 / 1e6); // pulldown, TODO: diode
}

double controlFunc(const double sensorValue) {
  return sensorValue + config.noControlValue - config.controlValue;
}
