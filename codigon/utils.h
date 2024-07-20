#ifndef UTILS_H
#define UTILS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"

#define MOTOR_Q 3

// Estructuras para almacenar los datos del sensor
struct Angles {
    float angleX;
    float angleY;
    float angleZ;
};

struct CircleAngles {
   int16_t motor_angle[MOTOR_Q]; 
};

// Declaraciones de funciones
String readAngles();
Angles getAngles();
void sendSpeedCommand(int motorIndex, float speed);
void sendSingleTurnCommand(int motorIndex, byte spin, float speed, float angler);
CircleAngles query();
void stop_motor(unsigned long motorID);
void open_gripper(int valvula, int compressor);
void stay_gripper(int valvula, int compressor); 
void close_gripper(int valvula, int compressor );
void shutdown_m(int motorIndex);
void sendActualTurnCommand(int motorIndex, float speed, float angler);
void actualtozero(int motorIndex);
void system_reset(int motorIndex);
int16_t angle_receive(int motorIndex);

#endif // UTILS_H

