#include <WiFi.h>
#include <HTTPClient.h>

// Configuración de la red WiFi
const char* ssid = "Octopus";
const char* password = "123456789";

// Dirección IP o nombre de dominio con la ruta URL
const char* serverName = "http://192.168.4.1/angles"; // URL para obtener los ángulos

// Variables para almacenar los datos del sensor
String anglex;
String angley;
String anglez;

unsigned long previousMillis = 0;
const long interval = 50 ; // Intervalo de actualización en milisegundos sincronizar   

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
}

void loop() {
  unsigned long currentMillis = millis();
  
  if (currentMillis % 100 == 0) { // Leer cada 100 ms
    String angles = readAngles();
    Serial.println("Ángulos recibidos: " + angles);

    // Separar los ángulos usando el punto y coma
    int delimiter1 = angles.indexOf(';');
    int delimiter2 = angles.indexOf(';', delimiter1 + 1);

    if (delimiter1 != -1 && delimiter2 != -1) {
      String angleX_str = angles.substring(0, delimiter1);
      String angleY_str = angles.substring(delimiter1 + 1, delimiter2);
      String angleZ_str = angles.substring(delimiter2 + 1);

      float angleX = angleX_str.toFloat();
      float angleY = angleY_str.toFloat();
      float angleZ = angleZ_str.toFloat();

      Serial.print("Ángulo X: ");
      Serial.println(angleX);
      Serial.print("Ángulo Y: ");
      Serial.println(angleY);
      Serial.print("Ángulo Z: ");
      Serial.println(angleZ);
    } else {
      Serial.println("Error: Datos recibidos no están en el formato esperado");
    }
  }
}

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

