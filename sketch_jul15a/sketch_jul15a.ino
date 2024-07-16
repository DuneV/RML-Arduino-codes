#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"

// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";

// Dirección IP o nombre de dominio con la ruta URL
const char* serverNameAngleX = "http://192.168.4.1/AngleX";
const char* serverNameAngleY = "http://192.168.4.1/AngleY";
const char* serverNameAngleZ = "http://192.168.4.1/AngleZ";

// Variables para almacenar los datos del sensor
float anglex, angley, anglez;
float prevAnglex = 0, prevAngley = 0, prevAnglez = 0;

// Intervalo de actualización en milisegundos
const long interval = 500;
unsigned long previousMillis = 0;

// MCP2515
const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN);
const unsigned long VELOCITY_COMMAND_IDS[] = {0x141, 0x142, 0x143};

void setup() {
    Serial.begin(115200);

    // Conectar a la red WiFi
    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
    while(WiFi.status() != WL_CONNECTED) { 
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Conectado a la red WiFi con la dirección IP: ");
    Serial.println(WiFi.localIP());

    // Inicializar MCP2515
    if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) {
        Serial.println("MCP2515 Inicializado correctamente!");
    } else {
        Serial.println("Error al inicializar MCP2515...");
        while (1);
    }
    CAN.setMode(MCP_NORMAL);
}

void loop() {
  
      if (Serial.available() > 0) {
        char receivedChar = Serial.read(); // Leer el carácter recibido
        if (receivedChar == 'C') { // Si el carácter es 'C'
            configureCANID(0x02); // Configurar el CANID a 0x02 como ejemplo
        }
    }
         
}

String httpGETRequest(const char* serverName) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, serverName);
    int httpResponseCode = http.GET();
    String payload = "--"; 
  
    if (httpResponseCode > 0) {
        Serial.print("Código de respuesta HTTP: ");
        Serial.println(httpResponseCode);
        payload = http.getString();
    } else {
        Serial.print("Codigo de error: ");
        Serial.println(httpResponseCode);
    }
    http.end();

    return payload;
}

void sendSpeedCommand(int motorIndex, float speed) {
    // Convertir velocidad a formato adecuado para el motor (ejemplo)
    int32_t speedInt = (int32_t)(speed * 100); // Convertir a unidad interna si es necesario
    
    // Preparar datos del comando
    byte commandData[8] = {
        0xA2, 0x00, 0x00, 0x00,
        (byte)(speedInt & 0xFF), 
        (byte)((speedInt >> 8) & 0xFF), 
        (byte)((speedInt >> 16) & 0xFF), 
        (byte)((speedInt >> 24) & 0xFF)
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de velocidad enviado correctamente al Motor ");
        Serial.println(motorIndex + 1);
    } else {
        Serial.print("Error al enviar comando de velocidad al Motor ");
        Serial.println(motorIndex + 1);
    }
}

void configureCANID(byte newCANID) {
    // Comando para configurar el CANID
    byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                           (byte) (newCANID >> 8)& 0xFF };
    
    if (CAN.sendMsgBuf(0x141, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de configuración CANID enviado correctamente. Nuevo CANID: ");
        Serial.println(newCANID, HEX);
    } else {
        Serial.println("Error al enviar comando de configuración CANID.");
    }
}
