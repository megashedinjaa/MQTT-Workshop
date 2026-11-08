#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "WIFI_PASSWORD";


const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

const char* MQTT_TOPIC = "pipitest";
String nome = "Nome_oh_rei";

WiFiClient picoClient;
PubSubClient mqttClient(picoClient);

void connectToWiFi() {
    Serial.printf("\n[WIFI] A ligar a: %s ", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" LIGADO!");
}

void reconnectMQTT() {

    String clientId = "PicoW_Client_" + nome;

     while (!mqttClient.connected()) {

        if (WiFi.status() != WL_CONNECTED) {
            connectToWiFi();
        }

        Serial.print("[MQTT] A ligar ao Broker...");

        if (mqttClient.connect(clientId.c_str())) {
            Serial.println(" Conectado!");
        } else {
            Serial.printf(" Falha (rc=%d). Nova tentativa...\n", mqttClient.state());
            delay(3000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Aguarda a abertura da porta série
    
     connectToWiFi();

    // Configura o servidor e a função de callback do PubSubClient
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {

    // Garante que se mantém ligado ao broker MQTT
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    
    // Mantém a escuta de pacotes ativa e processa o buffer de rede
    mqttClient.loop();
 

    static unsigned long lastPublish = 0;
    if (millis() - lastPublish >= 10000) {
        lastPublish = millis();

        mqttClient.publish(MQTT_TOPIC, "és um coco");
        } 
    }