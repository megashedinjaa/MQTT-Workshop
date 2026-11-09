#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID = "Pirocossauro";
const char* WIFI_PASS = "qwertyui";

const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

const char* MQTT_TOPIC = "pipitest/I202/distance";
String nome = "pico5";
String clientId = "PicoW_Client_" + nome;

// ------------------------CONFIGURAÇÃO do Sonar (HC-SR04)----------------------------------------------
// Pinos digitais para Trigger e Echo (ajusta aos pinos onde ligaste na placa)
const int TRIG_PIN = 14; 
const int ECHO_PIN = 15;
//------------------------------------------------------------------------------------------------------

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

// Reconnect com LWT 
void reconnectMQTT() {

    JsonDocument LWT;

    LWT["dispositivo"] = clientId;
    LWT["estado"] = "offline"; 
    LWT["local"] = "FEUP/I202";

    String lwt;
    serializeJson(LWT, lwt);

    JsonDocument back_online;

    back_online["dispositivo"] = clientId;
    back_online["estado"] = "online"; 
    back_online["local"] = "FEUP/I202";

    String online;
    serializeJson(back_online, online);

    while (!mqttClient.connected()) {
        if (WiFi.status() != WL_CONNECTED) {
            connectToWiFi();
        }
        Serial.print("[MQTT] A ligar ao Broker central...");

        if (mqttClient.connect(clientId.c_str(), MQTT_TOPIC, 1, true, lwt.c_str())) {
            Serial.println(" Conectado!");

            mqttClient.publish(MQTT_TOPIC, online.c_str(), true);
        } else {
            Serial.printf(" Falha (rc=%d). Nova tentativa...\n", mqttClient.state());
            delay(3000);
        }
    }
}

//------------------------------------Função do Sensor (Sonar)------------------------------------------
float lerDistanciaSonar() {
    // Garante que o pino Trigger começa a LOW
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    // Envia um pulso de 10 microssegundos no Trigger para disparar o ultrassom
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Lê o tempo (em microssegundos) que o pino Echo fica a HIGH
    // Timeout de 30000 us (~5 metros máx) para não bloquear o código se não houver obstáculo
    long duracao = pulseIn(ECHO_PIN, HIGH, 30000);

    // Se duracao for 0, significa que deu timeout (nenhum objeto detetado no alcance)
    if (duracao == 0) {
        return -1.0; 
    }

    // Distância em cm = (Tempo * Velocidade do Som [0.0343 cm/us]) / 2 (ida e volta)
    float distanciaCm = (duracao * 0.0343) / 2.0;

    return distanciaCm;
}

//------------------------------------------------------------------------------------------------------

// Criação do JSON "DOC"
void publicarJson(float distancia) {

    JsonDocument doc;

    doc["dispositivo"] = clientId;
    doc["distancia_cm"] = serialized(String(distancia, 2)); 
    doc["local"] = "FEUP/I202";

    String payload;
    serializeJson(doc, payload);

    mqttClient.publish(MQTT_TOPIC, payload.c_str());
    Serial.printf("[MQTT TX] Publicado: %s\n", payload.c_str());
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    // Configuração dos pinos do Sonar
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    connectToWiFi();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
 
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();

    static unsigned long lastPublish = 0;
    // Publicação a cada 500 ms (0,5 segundos)
    if (millis() - lastPublish >= 250) {
        lastPublish = millis();

        float distanciaReal = lerDistanciaSonar();
        publicarJson(distanciaReal);
    }
}