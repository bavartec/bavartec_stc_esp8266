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
  assert(word0.equals("device"));

  const int delimiter1 = topic.indexOf('/', delimiter0 + 1);
  assert(delimiter1 > 0);

  const String word1 = topic.substring(delimiter0 + 1, delimiter1);
  assert(word1.equals(config.mqtt.user));

  const String key = topic.substring(delimiter1 + 1);

  if (reconfig(key, value)) {
    save();
  }
}

void setupMQTT() {
  initMQTT();
}

void initMQTT() {
  static Client* wifiClient = nullptr;

  if (wifiClient) {
    delete wifiClient;
    wifiClient = nullptr;
  }

  if (config.mqtt.port == 8883 || config.mqtt.port == 443) {
    BearSSL::WiFiClientSecure* wifiClientSecure = new BearSSL::WiFiClientSecure();
    wifiClientSecure->setCertStore(&certStore);
    wifiClient = wifiClientSecure;
  } else {
    wifiClient = new WiFiClient();
  }

  mqttClient.setClient(*wifiClient);
  mqttClient.setServer(config.mqtt.server, config.mqtt.port);
  mqttClient.setCallback(mqttCallback);
}

void loopMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
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
    String topic = "device/";
    topic += config.mqtt.user;
    topic += "/#";
    mqttClient.subscribe(topic.c_str());
  }

  {
    String topic = "device/";
    topic += config.mqtt.user;
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

  Serial.print(F("Connecting MQTT client: "));
  Serial.println(config.mqtt.user);

  mqttClient.connect(WiFi.macAddress().c_str(), config.mqtt.user, config.mqtt.pass);
  lastConnect = millis();

  // randomized truncated binary exponential backoff
  mqttRetryWait = 1 << (10 + min(mqttRetryCount, 5));
  mqttRetryWait += RANDOM_REG32 % mqttRetryWait;
  mqttRetryCount++;
}
