#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi_name"
#define WIFI_PASS "wifi_password"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_ID "pico_name"
#define MQTT_TOPIC "ws-mqtt"
#define MQTT_SUB_TOPIC "ws-mqtt/#"
#define MQTT_REQ_TOPIC "ws-mqtt/requests"

WiFiClient wifi;
PubSubClient mqtt(wifi);

void callback(char *topic, uint8_t *payload, unsigned int length) {
  if (!strcmp(topic, MQTT_REQ_TOPIC)) {
    JsonDocument inDoc;
    DeserializationError err = deserializeJson(inDoc, payload, length);
    if (err) {
      Serial.print("Error: ");
      Serial.println(err.c_str());
      return;
    }
    const char *name = inDoc["name"] | "N/A";
    if (!strcmp(name, "N/A")) return;
    char nameTopic[128] = "";
    sprintf(nameTopic, "%s/led/%s", MQTT_TOPIC, name);
    JsonDocument outDoc;
    outDoc["name"] = MQTT_ID;
    outDoc["setLed"] = random(0, 2) == 1; // randomly set the LED state to true or false
    uint8_t buffer[128];
    size_t len = serializeJson(outDoc, buffer);
    mqtt.publish(nameTopic, buffer, len);
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
}