#include "utils.h"

const char* serverName = "http://192.168.4.1/angles";
const unsigned long VELOCITY_COMMAND_IDS[3] = {0x141, 0x142, 0x143};
const unsigned long RESPONSE_STATUS_ID[3] = {0x241, 0x242, 0x243};

extern MCP_CAN CAN;

// Per motor command

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
    delay(50);
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == RESPONSE_STATUS_ID[motorIndex]) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.print("Motor Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
            Serial.println(RESPONSE_STATUS_ID[motorIndex], HEX);
            
            Serial.print("Torque Current: ");
            Serial.print(torqueCurrent * 0.01); // Assuming 0.01A/LSB
            Serial.println(" A");
            
            Serial.print("Motor Speed: ");
            Serial.print(speed);
            Serial.println(" dps");
            
            Serial.print("Motor Angle: ");
            Serial.print(angle/100);
            Serial.println(" degrees");
        }
    } else {
        Serial.println("No Message Available");
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
        (byte)((angleInt >> 16) & 0xFF), 
        (byte)((angleInt >> 24) & 0xFF)
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de velocidad enviado correctamente al Motor ");
        Serial.println(motorIndex + 1);
    } else {
        Serial.print("Error al enviar comando de velocidad al Motor ");
        Serial.println(motorIndex + 1);
    }
    delay(50);
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == RESPONSE_STATUS_ID[motorIndex]) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.print("Motor Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
            Serial.println(RESPONSE_STATUS_ID[motorIndex], HEX);
            
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
    } else {
        Serial.println("No Message Available");
    }
    
}

void sendActualTurnCommand(int motorIndex, float speed, float angler) {
    int32_t speedInt = (int32_t)(speed * 100);
    int32_t angleInt = (int32_t)(angler * 100);

    byte commandData[8] = {
        0xA8, 0x00,
        (byte)(speedInt & 0xFF), 
        (byte)((speedInt >> 8) & 0xFF), 
        (byte)(angleInt & 0xFF), 
        (byte)((angleInt >> 8) & 0xFF),
        (byte)((angleInt >> 16) & 0xFF), 
        (byte)((angleInt >> 24) & 0xFF) 
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Comando de velocidad enviado correctamente al Motor ");
        Serial.println(motorIndex + 1);
    } else {
        Serial.print("Error al enviar comando de velocidad al Motor ");
        Serial.println(motorIndex + 1);
    }
    delay(50);
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId == RESPONSE_STATUS_ID[motorIndex]) {
            // Process the response
            int8_t temperature = rxBuf[1];
            int16_t torqueCurrent = (rxBuf[3] << 8) | rxBuf[2];
            int16_t speed = (rxBuf[5] << 8) | rxBuf[4];
            int16_t angle = (rxBuf[7] << 8) | rxBuf[6];
            
            Serial.print("Motor Temperature: ");
            Serial.print(temperature);
            Serial.println(" °C");
            Serial.println(RESPONSE_STATUS_ID[motorIndex], HEX);
            
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
    } else {
        Serial.println("No Message Available");
    }
    
}

CircleAngles query() {
    CircleAngles data;
    byte commandData[8] = {0x9C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    for (int i = 0; i < MOTOR_Q; i++)
    {
      CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[i], 0, 8, commandData);
      if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];
        CAN.readMsgBuf(&rxId, &len, rxBuf);
        Serial.print(rxId);
        data.motor_angle[i] = (rxBuf[7] << 8) | rxBuf[6];
    } else {
        Serial.println("No Message Available on" + String(VELOCITY_COMMAND_IDS[i]));
    }
    }
    return data;
}

int16_t angle_receive(int motorIndex)
{
  int16_t angle;
  byte commandData[8] = {0x94, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
 if (CAN.sendMsgBuf(RESPONSE_STATUS_ID[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Solicitud de angulo correcta ");
        Serial.println(motorIndex + 1);
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
        long unsigned int rxId;
        unsigned char len = 0;
        unsigned char rxBuf[8];
        CAN.readMsgBuf(&rxId, &len, rxBuf);
        angle = (rxBuf[7] << 8) | rxBuf[6];
    }
    else
    {
      angle = 0;
      }  
 } else {
        Serial.print("Error solicitud de angulo ");
        Serial.println(motorIndex + 1);
  }
  return angle;
}

void shutdown_m(int motorIndex)
{
   byte commandData[8] = {
        0x80, 0x00,
        0x00, 
        0x00, 
        0x00, 
        0x00,
        0x00, 0x00 
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Se apago el motor ");
        Serial.println(motorIndex + 1);
    }
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

void actualtozero(int motorIndex)
{
  byte commandData[8] = {
        0x64, 0x00,
        0x00, 
        0x00, 
        0x00, 
        0x00,
        0x00, 0x00 
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Se apago el motor ");
        Serial.println(motorIndex + 1);
    }
}

void system_reset(int motorIndex)
{
  byte commandData[8] = {
        0x76, 0x00,
        0x00, 
        0x00, 
        0x00, 
        0x00,
        0x00, 0x00 
    };
    
    if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[motorIndex], 0, 8, commandData) == CAN_OK) {
        Serial.print("Se apago el motor ");
        Serial.println(motorIndex + 1);
    }
}

// end-effector

void open_gripper(int valvula, int compressor)
{
  digitalWrite(valvula, LOW);
  digitalWrite(compressor, HIGH);
}
void stay_gripper(int valvula, int compressor)
{
  digitalWrite(valvula, LOW);
  digitalWrite(compressor, LOW);
}
void close_gripper(int valvula, int compressor )
{
  digitalWrite(valvula, HIGH);
  digitalWrite(compressor, LOW);
}

// multi-motor functions
