#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>


const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "wifi_pass";


const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

const char* MQTT_TOPIC = "pipitest";
String nome = "Nome_oh_rei";


WiFiClient picoClient;
PubSubClient mqttClient(picoClient);


//função de callback para não fazer fuckass polling
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("\n[MQTT RX] Mensagem recebida no tópico: %s\n", topic);
    Serial.print("[PAYLOAD] ");
    
    // Converte o payload (bytes) numa string legível para imprimir
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

}

//Funçãozinha muito importante
void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("[MQTT] A ligar ao Broker online...");
        
        //clientID para fins de organização
        String clientId = "PicoW_Client_" + nome;

            //CLientID é preciso aqui para o broker saber quem somos
        if (mqttClient.connect(clientId.c_str())) {

            // Subscreve novamente ao tópico após recuperar a ligação
            mqttClient.subscribe(MQTT_TOPIC);
            Serial.printf("[MQTT] Subscrito ao tópico: %s\n", MQTT_TOPIC);
        } else {
            Serial.printf(" Falha (rc=%d). Nova tentativa em 3 segundos...\n", mqttClient.state());
            delay(3000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Aguarda a abertura da porta série
    
     WiFi.begin(WIFI_SSID, WIFI_PASS);

     while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    // Configura o Broker, a Porta e a função de callback do PubSubClient
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(callback);
}

void loop() {
    // Garante que se mantém ligado ao broker MQTT
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    
    // Mantém a escuta de pacotes ativa e processa o buffer de rede
    mqttClient.loop();

    }