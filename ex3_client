#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi"
#define WIFI_PASS "vbcu6774"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_ID "bob"
#define MQTT_TOPIC "ws-mqtt"
#define MQTT_SUB_TOPIC "ws-mqtt/#"
#define MQTT_REQ_TOPIC "pipitest/requests"

WiFiClient wifi;
PubSubClient mqtt(wifi);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(callback);
}

void loop() {
  ensureWiFi();
  ensureMQTT();
  mqtt.loop();

  mqtt.publish("ws-mqtt/requests", "{\"name\":\"mary\"}");
  delay(1000);
}

void callback(char *topic, uint8_t *payload, unsigned int length) {
  if (!strcmp(topic, "ws-mqtt/led/mary")) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, payload, length);
    if (err) {
      Serial.print("Error: ");
      Serial.println(err.c_str());
      return;
    }
    bool ledBool = doc["setLed"] | false;
    digitalWrite(LED_BUILTIN, ledBool ? 1 : 0);
  }
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis(); // starts the timeout
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println(" CONNECTED TO WIFI!");
}

void ensureMQTT() {
  if (mqtt.connected()) return;

  while (!mqtt.connected()) {
    Serial.print(".");
    if (!mqtt.connect(MQTT_ID)) delay(1000);
  }
  Serial.println(" CONNECTED TO MQTT!");

  // only reached after a fresh connect, because of the early return above
  if (mqtt.subscribe(MQTT_SUB_TOPIC))
    Serial.println(" SUBSCRIBED!");
  else Serial.println(" SUBSCRIBE FAILED");
}
