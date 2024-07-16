#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"


#define MOTOR_Q 3

// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";

// Dirección IP o nombre de dominio con la ruta URL para obtener los ángulos
const char* serverName = "http://192.168.4.1/angles";

// Variables para almacenar los datos del sensor
struct angles {
    float angleX;
    float angleY;
    float angleZ;
};

struct CircleAngles {
    int16_t circleAngle[MOTOR_Q];
};

angles actualAngles;

// Intervalo de actualización en milisegundos
const long interval = 100;
unsigned long previousMillis = 0;

// MCP2515
const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN);
const unsigned long VELOCITY_COMMAND_IDS[3] = {0x141, 0x142, 0x143};
const unsigned long RESPONSE_STATUS_ID [3] = {0x241, 0x242, 0x243};
/*
   Funciones Utilitarias
*/

String readAngles() {
    WiFiClient client;
    HTTPClient http;
    String payload = "--";

    if (WiFi.status() == WL_CONNECTED) {
        // Realizar la solicitud HTTP GET
        http.begin(client, serverName);
        int httpResponseCode = http.GET();
        
        if (httpResponseCode > 0) {
            Serial.print("Código de respuesta HTTP: ");
            Serial.println(httpResponseCode);
            payload = http.getString();
        } else {
            Serial.print("Código de error HTTP: ");
            Serial.println(httpResponseCode);
        }
        // Liberar recursos
        http.end();
    } else {
        Serial.println("WiFi desconectado");
    }
    return payload;
}

angles getAngles() {
    String anglesStr = readAngles();
    angles data;
    int delimiter1 = anglesStr.indexOf(';');
    int delimiter2 = anglesStr.indexOf(';', delimiter1 + 1);
    
    if (delimiter1 != -1 && delimiter2 != -1) {
        String angleX_str = anglesStr.substring(0, delimiter1);
        String angleY_str = anglesStr.substring(delimiter1 + 1, delimiter2);
        String angleZ_str = anglesStr.substring(delimiter2 + 1);
        
        // Convertir las cadenas a valores flotantes
        data.angleX = angleX_str.toFloat();
        data.angleY = angleY_str.toFloat();
        data.angleZ = angleZ_str.toFloat();
    }
    return data;
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

CircleAngles query() {
    CircleAngles data;

    byte commandData[8] = {0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    CAN.sendMsgBuf(0x280, 0, 8, commandData);

    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];
        CAN.readMsgBuf(&rxId, &len, rxBuf);

        for (int i = 0; i < MOTOR_Q; i++) {
            if (rxId == RESPONSE_STATUS_ID[i]) {
                data.circleAngle[i] = (rxBuf[7] << 8) | rxBuf[6];
            }
        }
    } else {
        Serial.println("No Message Available");
    }

    return data;
}

void stop_motor(unsigned long motorID) {
    // Comando para leer el CANID actual
    byte commandData[8] = {0x81, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Enviar el mensaje con ID del motor
    if (CAN.sendMsgBuf(motorID, 0, 8, commandData) == CAN_OK) {
        Serial.print("Detenido motor: ");
        Serial.println(motorID, HEX);
    } else {
        Serial.println("Error *stop_motor* ");
    }
}

/*
   Funciones Arduino Core
*/

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
    unsigned long currentMillis = millis();
    
    // Actualizar ángulos a intervalos regulares
    if (currentMillis - previousMillis >= interval) {
        actualAngles = getAngles();
        previousMillis = currentMillis;
    }

    // Manejar comandos desde el puerto serial
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        
        switch (receivedChar) {
            case 'A':
                sendSpeedCommand(0, 5);
                break;
            case 'W':
                sendSpeedCommand(1, 5);
                break;
            case 'U':
                sendSpeedCommand(2, 5);
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


