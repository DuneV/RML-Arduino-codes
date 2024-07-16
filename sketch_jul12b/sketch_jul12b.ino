#include <SPI.h>
#include "mcp_can.h"

const int SPI_CS_PIN = 5; // Pin CS del MCP2515
MCP_CAN CAN(SPI_CS_PIN);

// CAN IDs for controlling the position and velocity of each motor
const unsigned long POSITION_COMMAND_IDS[] = {0x141, 0x142, 0x143};
const unsigned long VELOCITY_COMMAND_IDS[] = {0x151, 0x152, 0x153};

// CAN IDs for receiving the status response from each motor
const unsigned long RESPONSE_STATUS_IDS[] = {0x241, 0x242, 0x243}; 

const int MOTOR_COUNT = 3;

int16_t targetAngles[MOTOR_COUNT] = {90, 180, 270}; // Example target angles for three motors
int32_t velocities[MOTOR_COUNT] = {170000, 170000, 170000}; // Example velocities for three motors

void setup() {
    Serial.begin(115200);
    
    if (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) == CAN_OK) { // Set baud rate to 1 Mbps
        Serial.println("MCP2515 Initialized Successfully!");
    } else {
        Serial.println("Error Initializing MCP2515...");
        while (1); // Detiene el programa si hay un error en la inicialización
    }

    CAN.setMode(MCP_NORMAL);
}

void loop() {
    for (int i = 0; i < MOTOR_COUNT; i++) {
        // Prepare the command data for setting motor position
        byte positionCommandData[8] = {
            0xA4, // Command byte for position control (example)
            (byte)(targetAngles[i] & 0xFF), 
            (byte)((targetAngles[i] >> 8) & 0xFF),
            0x00, 0x00, 0x00, 0x00, 0x00 // Remaining bytes reserved
        };
        
        // Send position command
        if (CAN.sendMsgBuf(POSITION_COMMAND_IDS[i], 0, 8, positionCommandData) == CAN_OK) {
            Serial.print("Position Command Sent Successfully to Motor ");
            Serial.println(i + 1);
        } else {
            Serial.print("Error Sending Position Command to Motor ");
            Serial.println(i + 1);
        }

        // Prepare the command data for setting motor velocity
        byte velocityCommandData[8] = {
            0xA5, // Command byte for velocity control (example)
            (byte)(velocities[i] & 0xFF), 
            (byte)((velocities[i] >> 8) & 0xFF), 
            (byte)((velocities[i] >> 16) & 0xFF), 
            (byte)((velocities[i] >> 24) & 0xFF),
            0x00, 0x00, 0x00 // Remaining bytes reserved
        };
        
        // Send velocity command
        if (CAN.sendMsgBuf(VELOCITY_COMMAND_IDS[i], 0, 8, velocityCommandData) == CAN_OK) {
            Serial.print("Velocity Command Sent Successfully to Motor ");
            Serial.println(i + 1);
        } else {
            Serial.print("Error Sending Velocity Command to Motor ");
            Serial.println(i + 1);
        }
    }

    delay(500); // Adjust the delay as needed for the desired update rate
}

