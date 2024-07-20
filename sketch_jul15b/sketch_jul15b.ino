#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"

// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";

// MCP2515
const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
    Serial.begin(115200);

    // Conectar a la red WiFi
    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) { 
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
        if (receivedChar == 'R') { // Si el carácter es 'R'
            readMotorID(0x141); // Leer el CANID del motor con ID actual 0x141
        }
        if (receivedChar == 'Y') { // Si el carácter es 'R'
            configureCANID(); 
        }
        if (receivedChar == 'E') { // Si el carácter es 'R'
            readMotorID(0x142); // Leer el CANID del motor con ID actual 0x141
        }
        if (receivedChar == 'T') { // Si el carácter es 'R'
            readMotorID(0x143); // Leer el CANID del motor con ID actual 0x141
        }
        if (receivedChar == 'U')
        {
            multiencoder();  
        }
        if (receivedChar == 'H')
        {
          unableff();
          }
        if (receivedChar == 'P')
        {
          configureCANID_R();
         }
    }

    // Leer mensajes de retorno
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxID;
        unsigned char len;
        unsigned char buf[8];
        
        CAN.readMsgBuf(&rxID, &len, buf);
        Serial.print("Mensaje CAN recibido con ID: ");
        Serial.println(rxID, HEX);
        for (int i = 0; i < len; i++) {
            Serial.print(buf[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}

void readMotorID(unsigned long motorID) {
    // Comando para leer el CANID actual
    byte commandData[8] = {0x79, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Enviar el mensaje con ID del motor
    if (CAN.sendMsgBuf(0x300, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de lectura de CANID enviado correctamente al Motor con ID: ");
        Serial.println(motorID, HEX);
    } else {
        Serial.println("Error al enviar comando de lectura de CANID.");
    }
}
void configureCANID() {
    // Comando para configurar el CANID
    byte commandData[8] = {0x79, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03 };
    
    if (CAN.sendMsgBuf(0x300, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de configuración CANID enviado correctamente. Nuevo CANID: ");
        Serial.println(0x03, HEX);
    } else {
        Serial.println("Error al enviar comando de configuración CANID.");
    }
}

void configureCANID_R() {
    // Comando para configurar el CANID
    byte commandData[8] = {0xB6, 0x60, 0x01, 0x01, 0x00, 0x00, 0x00, 0x03 };
    
    if (CAN.sendMsgBuf(0x141, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de configuración CANID enviado correctamente");
        Serial.println(0x03, HEX);
    } else {
        Serial.println("Error al enviar comando de configuración CANID.");
    }
}

void multiencoder()
{
   byte commandData[8] = {0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    if (CAN.sendMsgBuf(0x280, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de configuración MULTI: ");
        Serial.println(0x280, HEX);
    } else {
        Serial.println("Error al enviar comando de MULTI.");
    }
  
  }

void unableff()
{
  byte commandData[8] = {0x20, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    
    if (CAN.sendMsgBuf(0x141, 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de filtro: ");
        Serial.println(0x141, HEX);
    } else {
        Serial.println("Error al deshabilitar.");
    }
}
