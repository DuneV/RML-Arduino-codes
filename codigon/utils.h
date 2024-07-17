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
    int16_t circleAngle[MOTOR_Q];
};

// Declaraciones de funciones
String readAngles();
Angles getAngles();
void sendSpeedCommand(int motorIndex, float speed);
void sendSingleTurnCommand(int motorIndex, byte spin, float speed, float angler);
CircleAngles query();
void stop_motor(unsigned long motorID);

#endif // UTILS_H

