#define MQTT_MAX_PACKET_SIZE 512
#define MQTT_KEEPALIVE 60
#define MQTT_SOCKET_TIMEOUT 60

#include <PubSubClient.h>

const __FlashStringHelper* stateName(const int state) {
  switch (state) {
    case -4: return F("MQTT_CONNECTION_TIMEOUT");
    case -3: return F("MQTT_CONNECTION_LOST");
    case -2: return F("MQTT_CONNECT_FAILED");
    case -1: return F("MQTT_DISCONNECTED");
    case 0: return F("MQTT_CONNECTED");
    case 1: return F("MQTT_CONNECT_BAD_PROTOCOL");
    case 2: return F("MQTT_CONNECT_BAD_CLIENT_ID");
    case 3: return F("MQTT_CONNECT_UNAVAILABLE");
    case 4: return F("MQTT_CONNECT_BAD_CREDENTIALS");
    case 5: return F("MQTT_CONNECT_UNAUTHORIZED");
  }

  return F("MQTT_UNKNOWN");
}

int mqttRetryCount = 0;
unsigned long mqttRetryWait = 0;

PubSubClient mqttClient;
WiFiClient* mqttWifiClient = nullptr;

void resetMQTT() {
  mqttRetryCount = 0;
  mqttRetryWait = 0;
  mqttClient.disconnect();

  initMQTT();
}

void mqttCallback(const char* topicRaw, const uint8_t* payload, const unsigned int length) {
  const String topic = String(topicRaw);

  char valueRaw[length + 1];
  memcpy(valueRaw, payload, length);
  valueRaw[length] = '\0';
  const String value = String(valueRaw);

  const int delimiter0 = topic.indexOf('/');
  assert(delimiter0 > 0);

  const String word0 = topic.substring(0, delimiter0);
  assert(word0.equals("user"));

  const int delimiter1 = topic.indexOf('/', delimiter0 + 1);
  assert(delimiter1 > 0);

  const String word1 = topic.substring(delimiter0 + 1, delimiter1);
  assert(word1.equals(config.mqtt.user));

  const int delimiter2 = topic.indexOf('/', delimiter1 + 1);
  assert(delimiter2 > 0);

  const String word2 = topic.substring(delimiter1 + 1, delimiter2);
  assert(word2.equals("device"));

  const int delimiter3 = topic.indexOf('/', delimiter2 + 1);
  assert(delimiter3 > 0);

  const String word3 = topic.substring(delimiter2 + 1, delimiter3);
  assert(word3.equals(WiFi.macAddress()));

  const String key = topic.substring(delimiter3 + 1);

  if (reconfig(key, value)) {
    save();
  }
}

void setupMQTT() {
  initMQTT();
}

void initMQTT() {
  if (mqttWifiClient && mqttWifiClient != &wifiClientSecure) {
    delete mqttWifiClient;
    mqttWifiClient = nullptr;
  }

  if (config.mqtt.port == 8883 || config.mqtt.port == 443) {
    mqttWifiClient = &wifiClientSecure;
  } else {
    mqttWifiClient = new WiFiClient();
  }

  mqttClient.setClient(*mqttWifiClient);
  mqttClient.setServer(config.mqtt.server, config.mqtt.port);
  mqttClient.setCallback(mqttCallback);
}

void loopMQTT() {
  if (!wifiAsUsual()) {
    return;
  }

  loopMQTT_STA();

  if (!mqttClient.loop()) {
    return;
  }

  static unsigned long lastPublish = 1E9;
  const unsigned long time = millis();

  if (time - lastPublish < 60 * 1000) {
    return;
  }

  {
    String topic = "user/";
    topic += config.mqtt.user;
    topic += "/device/";
    topic += WiFi.macAddress();
    topic += "/#";
    mqttClient.subscribe(topic.c_str());
  }

  {
    String topic = "user/";
    topic += config.mqtt.user;
    topic += "/device/";
    topic += WiFi.macAddress();
    topic += "/sensorTemperature";

    const String payload = to_string(sensorTemperature, 7, "%.2f");

    mqttClient.publish(topic.c_str(), payload.c_str());
    lastPublish = time;
  }
}

void loopMQTT_STA() {
  static int lastState = MQTT_DISCONNECTED;
  static unsigned long lastConnect = 1E9;

  const int state = mqttClient.state();
  const unsigned long time = millis();

  if (lastState != state) {
    Serial.println(stateName(state));

    if (state == MQTT_CONNECTED) {
      mqttRetryCount = 0;
      mqttRetryWait = 0;
    }
  }

  lastState = state;

  if (config.mqtt.user[0] == '\0' || state == MQTT_CONNECTED || time - lastConnect < mqttRetryWait) {
    return;
  }

  Serial.print(F("Connecting to MQTT: "));
  Serial.println(config.mqtt.user);

  if (config.mqtt.port == 8883 || config.mqtt.port == 443) {
    setClock();
  }

  if (mqttWifiClient == &wifiClientSecure && wifiClientSecure.probeMaxFragmentLength(config.mqtt.server, config.mqtt.port, 1024)) {
    wifiClientSecure.setBufferSizes(1024, 1024);
  }

  mqttClient.connect(WiFi.macAddress().c_str(), config.mqtt.user, config.mqtt.pass);
  lastConnect = millis();

  // randomized truncated binary exponential backoff
  mqttRetryWait = 1 << (10 + min(mqttRetryCount, 5));
  mqttRetryWait += RANDOM_REG32 % mqttRetryWait;
  mqttRetryCount++;
}
