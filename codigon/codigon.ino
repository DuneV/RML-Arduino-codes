#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"
#include "utils.h"

#define VEL 60

// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";

Angles actualAngles;

const long interval = 100;
unsigned long previousMillis = 0;

const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Conectado a la red WiFi con la dirección IP: ");
    Serial.println(WiFi.localIP());

    if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println("MCP2515 Inicializado correctamente!");
    } else {
        Serial.println("Error al inicializar MCP2515...");
        while (1);
    }
    CAN.setMode(MCP_NORMAL);
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        actualAngles = getAngles();
        Serial.println(actualAngles.angleX);
        Serial.println(actualAngles.angleY);
        Serial.println(actualAngles.angleZ);
        previousMillis = currentMillis;
    }

    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        
        switch (receivedChar) {
            case 'A':
                sendSpeedCommand(0, VEL);
                break;
            case 'W':
                sendSpeedCommand(1, VEL);
                break;
            case 'U':
                sendSpeedCommand(2, VEL);
                break;
            case 'S':
                sendSpeedCommand(0, 0);
                sendSpeedCommand(1, 0);
                sendSpeedCommand(2, 0);
                break;
            default:
                break;
        }
    }
}

