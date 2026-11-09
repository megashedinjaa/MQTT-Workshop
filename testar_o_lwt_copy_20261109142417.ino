#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Definições de rede e MQTT
#define WIFI_SSID "iPhone"
#define WIFI_PASS "POBREPLEBE3"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_ID "xXpicowXx"
#define LWT_TOPIC "pipitest"

WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long lastMsgTime =0;

void connectToWiFi() {
  Serial.printf("\n[WIFI] Connecting to: %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED!");
}

void reconnectMQTT() {
  while (!mqtt.connected()) {
    if (WiFi.status() != WL_CONNECTED) {
      connectToWiFi();
    }

    Serial.print("[MQTT] Connecting to the Broker with LWT...");

    // Parâmetros do LWT (Last Will and Testament)
    const char* willTopic = LWT_TOPIC;
    uint8_t willQoS = 1;
    bool willRetain = true;
    const char* willMessage = "OFFLINE (Ups net de pobre foi embora)";

    // Passar o LWT diretamente no método connect()
    if (mqtt.connect(MQTT_ID, willTopic, willQoS, willRetain, willMessage)) {
      Serial.println(" CONNECTED!");

      // Envia uma mensagem "ONLINE" assinalando o arranque normal do microcontrolador
      mqtt.publish(LWT_TOPIC, "ONLINE", true);
    } else {
      Serial.printf(" Failed (rc=%d). Retrying...\n", mqtt.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setKeepAlive(3);
}

void loop() {
  if (!mqtt.connected()) {
    reconnectMQTT();
  }
  mqtt.loop();
  unsigned long now = millis();
  if (now - lastMsgTime >= 3000) {
    lastMsgTime = now;

    String heartbeatMsg = "Ainda conectado!";
    Serial.println("A enviar heartbeat: " + heartbeatMsg);
    
    mqtt.publish(LWT_TOPIC, heartbeatMsg.c_str());
  }
}