#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define WIFI_SSID "wifi"
#define WIFI_PASS "password"
#define MQTT_BROKER "broker.hivemq.com"
#define MQTT_PORT 1883
#define MQTT_ID "picow-1234"
#define MQTT_PUB_TOPIC "topic"
#define MQTT_SUB_TOPIC "topic"

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

// Função de callback otimizada para receber e ler JSON
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.printf("\n[MQTT RX] Mensagem recebida no tópico: %s\n", topic);
    
    // 1. Aloca um JsonDocument (tamanho dinâmico na stack/heap adequado para a mensagem)
    JsonDocument doc;

    // 2. Deserializa o payload recebido (bytes) diretamente para JSON
    DeserializationError error = deserializeJson(doc, payload, length);

    // 3. Verifica se o JSON é válido
    if (error) {
        Serial.print("[JSON ERROR] Falha ao parsear o JSON: ");
        Serial.println(error.c_str());
        return; // Sai da função se o JSON estiver mal formatado
    }

    // 4. Extrai os dados do JSON (Exemplo: se mandares {"comando":"ligar","valor":42})
    // Podes alterar as chaves conforme o que fores enviar para a placa
    const char* comando = doc["comando"] | "desconhecido"; 
    int valor = doc["valor"] | 0;                        

    Serial.printf("[JSON] Comando recebido: %s | Valor: %d\n", comando, valor);

    // Exemplo de ação com base no JSON recebido:
    if (strcmp(comando, "ligar") == 0) {
        Serial.println("[AÇÃO] Comando para LIGAR executado!");
        // digitalWrite(LED_BUILTIN, HIGH);
    } else if (strcmp(comando, "desligar") == 0) {
        Serial.println("[AÇÃO] Comando para DESLIGAR executado!");
        // digitalWrite(LED_BUILTIN, LOW);
    }
}

// Função de reconexão MQTT
void reconnectMQTT() {
    while (!mqttClient.connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            connectToWiFi();
        }

        Serial.print("[MQTT] A ligar ao Broker online...");

        if (mqttClient.connect(MQTT_ID)) {
            Serial.println(" Conectado!");
            // Subscreve ao tópico para começar a receber os JSONs
            mqttClient.subscribe(MQTT_SUB_TOPIC);
            Serial.printf("[MQTT] Subscrito ao tópico: %s\n", MQTT_SUB_TOPIC);
        } else {
            Serial.printf(" Falha (rc=%d). Nova tentativa em 3 segundos...\n", mqttClient.state());
            delay(3000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000); // Aguarda a abertura da porta série
    
    connectToWiFi();

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