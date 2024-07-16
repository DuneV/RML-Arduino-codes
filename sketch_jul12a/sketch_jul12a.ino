#include <SPI.h>
#include "mcp_can.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de la red WiFi
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Dirección IP o nombre de dominio con la ruta URL
const char* serverNameAngleX = "http://192.168.4.1/AngleX";
const char* serverNameAngleY = "http://192.168.4.1/AngleY";
const char* serverNameAngleZ = "http://192.168.4.1/AngleZ";

// Variables para almacenar los datos del sensor
String anglex;
String angley;
String anglez;

unsigned long previousMillis = 0;
const long interval = 1000; // Intervalo de actualización en milisegundos sincronizar   


const int SPI_CS_PIN = 5; // Pin CS del MCP2515
MCP_CAN CAN(SPI_CS_PIN);

// CAN IDs for controlling the position of each motor
const unsigned long POSITION_COMMAND_IDS[] = {0x141, 0x142, 0x143}; 
// CAN IDs for receiving the status response from each motor
const unsigned long RESPONSE_STATUS_IDS[] = {0x241, 0x242, 0x243}; 

const int MOTOR_COUNT = 3;

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Tu nombre de dominio con la ruta URL o dirección IP con la ruta
  http.begin(client, serverName);
  
  // Enviar solicitud HTTP GET
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
  // Liberar recursos
  http.end();

  return payload;
}

void setup() {
    Serial.begin(115200);
    // Conectar a la red WiFi
    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
    while(WiFi.status() != WL_CONNECTED) { 
      delay(500);
    Serial.print(".");
  }
   if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) { // Set baud rate to 1 Mbps
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("Error Initializing MCP2515...");
        while (1); // Detiene el programa si hay un error en la inicialización
    }

    CAN.setMode(MCP_NORMAL);
    Serial.println("\nConnected to the WiFi network");
    Serial.print("Local ESP32 IP: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Command to set motor position
    int16_t positionSetpoints[MOTOR_COUNT] = {90, 180, 270}; // Example angle values for three motors, change as needed
    for (int i = 0; i < MOTOR_COUNT; i++) {
        // Prepare the command data for setting motor position
        byte commandData[8] = {0xA4, 0x00, 0x00, 0x00,
                               (byte)(positionSetpoints[i] & 0xFF), 
                               (byte)((positionSetpoints[i] >> 8) & 0xFF), 
                               0x00, 0x00}; // Assuming a simple command format for position control
        
        if (CAN.sendMsgBuf(POSITION_COMMAND_IDS[i], 0, 8, commandData) == CAN_OK) {
            Serial.print("Position Command Sent Successfully to Motor ");
            Serial.println(i + 1);
        } else {
            Serial.print("Error Sending Position Command to Motor ");
            Serial.println(i + 1);
        }
    }

    // Wait for motor's response
    while (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        for (int j = 0; j < MOTOR_COUNT; j++) {
            if (rxId == RESPONSE_STATUS_IDS[j]) {
                // Process the response
                int8_t temperature = rxBuf[1];
                int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
                int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
                int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
                
                Serial.print("Motor ");
                Serial.print(j + 1);
                Serial.print(" Temperature: ");
                Serial.print(temperature);
                Serial.println(" °C");
                
                Serial.print("Torque Current: ");
                Serial.print(torqueCurrent * 0.01); // Assuming 0.01A/LSB
                Serial.println(" A");
                
                Serial.print("Motor Speed: ");
                Serial.print(speed);
                Serial.println(" dps");
                
                Serial.print("Motor Angle: ");
                Serial.print(angle);
                Serial.println(" degrees");
            }
        }
    }

    delay(500); // Adjust the delay as needed for the desired update rate
}

