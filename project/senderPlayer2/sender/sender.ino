const int irLedPin = 4; // Steuert den Transistor

void setup() {
  pinMode(irLedPin, OUTPUT);
}

void loop() {
  digitalWrite(irLedPin, HIGH); // IR-LED an
  delay(100);                   // 100 ms leuchten
  digitalWrite(irLedPin, LOW);  // IR-LED aus
  delay(100);                   // 100 ms Pause
}
