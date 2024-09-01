const int relay = 26;
const int relay2 = 27; 

void setup() {
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
}

void loop() {
  // Normally Open configuration, send LOW signal to let current flow
  // (if you're usong Normally Closed configuration send HIGH signal)
  digitalWrite(relay, LOW);
  digitalWrite(relay2, LOW);
  Serial.println("Current Flowing");
  delay(500); 
  
  // Normally Open configuration, send HIGH signal stop current flow
  // (if you're usong Normally Closed configuration send LOW signal)
  digitalWrite(relay, HIGH);
  digitalWrite(relay2, HIGH);
  Serial.println("Current not Flowing");
  delay(500);
}

