#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include "mcp_can.h"
#include "utils.h"

#define VEL 40

const unsigned long VELOCITY_COMMAND_IDS[3] = {0x141, 0x142, 0x143};
// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";
const int valvula = 26; 
const int compressor =  27;

// Constantes de funcionamiento de motores

float current_anglem1;
float current_anglem2;
float current_anglem3;

// Factor proporcional 

float k_m1 = 1.0;
float k_m2 = 1.0;
float k_m3 = 1.0;

// Angulos iniciales 

float z_angle_init;
float x_angle_init;
float y_angle_init; 

// angulo actual

float current_angle_x;
float current_angle_y;
float current_angle_z;

Angles actualAngles;

const long interval = 100;
unsigned long previousMillis = 0;

const int SPI_CS_PIN = 5;
MCP_CAN CAN(SPI_CS_PIN);
void setup_motor(){
  actualtozero(0);
  actualtozero(1);
  actualtozero(2);
  system_reset(0);
  system_reset(1);
  system_reset(2); 
}
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
    setup_motor(); 
    actualAngles = getAngles(); 
    x_angle_init = actualAngles.angleX;
    y_angle_init = actualAngles.angleY;
    z_angle_init = actualAngles.angleZ;
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        //actualAngles = getAngles();
        //Serial.println(actualAngles.angleX);
        //Serial.println(actualAngles.angleY);
        //Serial.println(actualAngles.angleZ);
        previousMillis = currentMillis;
    }

    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        
        switch (receivedChar) {
            case 'V':
                actualtozero(0);
                actualtozero(1);
                actualtozero(2);
                break;
            case 'H':
                 system_reset(0);
                 system_reset(1);
                 system_reset(2);                 
            case 'O':
                shutdown_m(0);
                break;
            case 'T':
                sendActualTurnCommand(0, 2*VEL, -10);
                break;
            case 'K':
                query();
                break;
            case 'L':
                Serial.print(angle_receive(1));
                break;
            case 'I':
                stop_motor(VELOCITY_COMMAND_IDS[0]);
                stop_motor(VELOCITY_COMMAND_IDS[1]);
                stop_motor(VELOCITY_COMMAND_IDS[2]);
                break;
            case 'A':
                sendSpeedCommand(0, VEL);
                break;
            case 'B':
                sendSpeedCommand(0, -VEL);
                break;    
            case 'W':
                sendSpeedCommand(1, VEL);
                break;
            case 'E':
                sendSpeedCommand(1, -VEL);
                break;
            case 'U':
                sendSpeedCommand(2, VEL);
                break;
            case 'Y':
                sendSpeedCommand(2, -VEL);
                break;
            case 'F':
                shutdown_m(0);
                shutdown_m(1);
                shutdown_m(2);
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

