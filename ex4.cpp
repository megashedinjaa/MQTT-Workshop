#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

const char* WIFI_SSID = "SSID";
const char* WIFI_PASS = "WIFI_PASSWORD";

const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

const char* MQTT_TOPIC = "pipitest/I205/sensor";
String nome = "Nome_oh_rei";

// ------------------------CONFIGURAÇÃO do Termístor-----------------------------------------------------
// Pino ADC ligado ao divisor de tensão do termístor (GPIO 4 na ESP32-S3 / GPIO 26 no Pico W)
//const int TERMISTOR_PIN = 4;
const int TERMISTOR_PIN = 26;

// Parâmetros do Termístor NTC
const float NOMINAL_RESISTANCE = 10000.0; // Resistência a 25ºC (10k)
const float NOMINAL_TEMPERATURE = 25.0;   // Temperatura nominal em ºC
const float BETA_COEFFICIENT = 3950.0;    // Coeficiente Beta do NTC
const float SERIES_RESISTOR = 10000.0;    // Resistência fixa do divisor (10k)
//-------------------------------------------------------------------------------------------------

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

//Reconnect com LWT 
void reconnectMQTT() {
    String clientId = "PicoW_Client_" + nome;

    JsonDocument LWT;

    LWT["dispositivo"] = "PICO_W";
    LWT["estado"] = "offline"; 
    LWT["local"] = "FEUP/I205";

    String lwt;
    serializeJson(LWT, lwt);

     JsonDocument back_online;

    back_online["dispositivo"] = "PICO_W";
    back_online["estado"] = "online"; 
    back_online["local"] = "FEUP/I205";

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

//------------------------------------Função do Sensor--------------------------------------------------
float lerTemperaturaTermistor() {
    long somaLeituras = 0;
    int numAmostras = 10;
    for (int i = 0; i < numAmostras; i++) {
        somaLeituras += analogRead(TERMISTOR_PIN);
        delay(10);
    }
    float adcValue = somaLeituras / (float)numAmostras;

    // Proteção contra divisão por zero ou leitura a zero
    if (adcValue >= 4095) adcValue = 4094;
    if (adcValue <= 0) adcValue = 1;

    // Calcula a resistência do termístor com base no divisor de tensão
    float resistance = SERIES_RESISTOR * (4095.0 / adcValue - 1.0);

    // Equação de Steinhart-Hart simplificada (Beta Parameter Equation)
    float steinhart;
    steinhart = resistance / NOMINAL_RESISTANCE;       // (R/Ro)
    steinhart = log(steinhart);                        // ln(R/Ro)
    steinhart /= BETA_COEFFICIENT;                     // 1/B * ln(R/Ro)
    steinhart += 1.0 / (NOMINAL_TEMPERATURE + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                       // Inverte para Kelvin
    steinhart -= 273.15;                               // Converte para Celsius

    return steinhart;
}

//-------------------------------------------------------------------------------------------------------------

// Criação do JSON "DOC"
void publicarJson(float steinhart) {

    JsonDocument doc;

    doc["dispositivo"] = "PICO_W";
    doc["temperatura"] = serialized(String(steinhart, 2)); 
    doc["local"] = "FEUP/I205";

    String payload;
    serializeJson(doc, payload);

    mqttClient.publish(MQTT_TOPIC, payload.c_str());
    Serial.printf("[MQTT TX] Publicado: %s\n", payload.c_str());
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    analogReadResolution(12);

    connectToWiFi();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
 
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();

    static unsigned long lastPublish = 0; // <-- Faltava esta linha!
    if (millis() - lastPublish >= 10000) {
        lastPublish = millis();

        float temperaturaReal = lerTemperaturaTermistor();
        publicarJson(temperaturaReal);
    }
}