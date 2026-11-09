#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi_name"
#define WIFI_PASS "wifi_pass"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_ID "piconame"
#define MQTT_PUB_TOPIC "ws-mqtt/demo"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  ensureWiFi();
  ensureMQTT();
  mqttClient.loop();

  // Publicação de ambas as mensagens através do mesmo cliente ativo
  mqttClient.publish(MQTT_PUB_TOPIC, "o nosso grupo é o melhor ");
  delay(5000);
}

void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }
  }
  Serial.println(" CONNECTED TO WIFI!");
}

void ensureMQTT() {
  if (mqttClient.connected()) return;

  while (!mqttClient.connected()) {
    Serial.print(".");
    if (!mqttClient.connect(MQTT_ID)) delay(1000);
  }
  Serial.println(" CONNECTED TO MQTT!");
}
