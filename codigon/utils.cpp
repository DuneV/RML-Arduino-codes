#include "utils.h"

const char* serverName = "http://192.168.4.1/angles";
const unsigned long VELOCITY_COMMAND_IDS[3] = {0x141, 0x142, 0x143};
const unsigned long RESPONSE_STATUS_ID[3] = {0x241, 0x242, 0x243};

extern MCP_CAN CAN;

String readAngles() {
    WiFiClient client;
    HTTPClient http;
    String payload = "--";

    if (WiFi.status() == WL_CONNECTED) {
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
        http.end();
    } else {
        Serial.println("WiFi desconectado");
    }
    return payload;
}

Angles getAngles() {
    String anglesStr = readAngles();
    Angles data;
    int delimiter1 = anglesStr.indexOf(';');
    int delimiter2 = anglesStr.indexOf(';', delimiter1 + 1);
    
    if (delimiter1 != -1 && delimiter2 != -1) {
        String angleX_str = anglesStr.substring(0, delimiter1);
        String angleY_str = anglesStr.substring(delimiter1 + 1, delimiter2);
        String angleZ_str = anglesStr.substring(delimiter2 + 1);
        
        data.angleX = angleX_str.toFloat();
        data.angleY = angleY_str.toFloat();
        data.angleZ = angleZ_str.toFloat();
    }
    return data;
}

void sendSpeedCommand(int motorIndex, float speed) {
    int32_t speedInt = (int32_t)(speed * 100);

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

void sendSingleTurnCommand(int motorIndex, byte spin, float speed, float angler) {
    int32_t speedInt = (int32_t)(speed * 100);
    int32_t angleInt = (int32_t)(angler * 100);

    byte commandData[8] = {
        0xA6, spin,
        (byte)(speedInt & 0xFF), 
        (byte)((speedInt >> 8) & 0xFF), 
        (byte)(angleInt & 0xFF), 
        (byte)((angleInt >> 8) & 0xFF),
        0x00, 0x00 
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
    byte commandData[8] = {0x81, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (CAN.sendMsgBuf(motorID, 0, 8, commandData) == CAN_OK) {
        Serial.print("Detenido motor: ");
        Serial.println(motorID, HEX);
    } else {
        Serial.println("Error *stop_motor* ");
    }
}

