// === Roller Control Using BTN7960 ===

const int RPWM = 5;   // Motor forward (downward roll)
const int LPWM = 4;   // Motor reverse (upward roll)

void setup() {
  Serial.begin(9600);

  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);

  Serial.println("BTN7960 Roller Control Ready");
  Serial.println("Send 'r' to run rollers, 's' to stop.");
}

void rollers() {
  // Rollers run downward
  analogWrite(RPWM, 255);   // Max forward speed
  analogWrite(LPWM, 0);     
  delay(450);              // Let ball bounce

  // Reverse rollers to catch ball
  analogWrite(RPWM, 0);     
  analogWrite(LPWM, 255);   // Max reverse speed
  delay(750);

  stop();                   // Hold ball
  delay(2000);
}

void stop() {
  analogWrite(RPWM, 0);
  analogWrite(LPWM, 0);
}

void loop() {
  if (Serial.available()) {

    char ch = Serial.read();

    if (ch == 'r') {
      Serial.println("Starting rollers...");
      rollers();
    } else if (ch == 's') {
      Serial.println("Stopping rollers.");
      stop();
    }
  }
}
