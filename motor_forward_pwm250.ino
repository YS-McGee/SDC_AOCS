const int pwmPin = 10;
const int enablePin = 9;
const int directionPin = 8;

void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(directionPin, OUTPUT);

  digitalWrite(directionPin, HIGH);   // Set reverse direction (LOW for reverse counter-clockwise)
  digitalWrite(enablePin, HIGH);     // Enable motor driver
  analogWrite(pwmPin, 250);          // High PWM = Low speed for Nidec motor
}

void loop() {
  // Motor spins continuously in forward
}
