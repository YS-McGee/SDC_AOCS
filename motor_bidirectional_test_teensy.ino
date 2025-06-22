const int pwmPin = 10;       // PWM output to motor (white wire)
const int enablePin = 9;     // Enable pin (yellow wire)
const int directionPin = 8;  // Direction pin (blue wire)

// 10% of 255 (8-bit) ≈ 25
const int lowSpeedPWM = 25;   // Very high speed

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("Motor Ultra-Slow Direction Test: 2% speed FORWARD → STOP → REVERSE");

  pinMode(pwmPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(directionPin, OUTPUT);

  analogWriteResolution(8);              // 8-bit PWM: 0–255
  analogWriteFrequency(pwmPin, 25000);   // 25kHz for Nidec

  // Ensure motor is off initially
  analogWrite(pwmPin, 0);
  digitalWrite(enablePin, LOW);
}

void loop() {
  // === FORWARD ===
  Serial.println("Direction: FORWARD, Speed: 2%");
  digitalWrite(directionPin, HIGH);  // Forward
  delay(100);                        // Direction settle time
  digitalWrite(enablePin, HIGH);     // Enable motor
  analogWrite(pwmPin, lowSpeedPWM);
  delay(5000);                       // Let it spin forward

  // === STOP ===
  // Serial.println("Stopping motor");
  // analogWrite(pwmPin, 0);
  // digitalWrite(enablePin, LOW);
  // delay(3000);

  // === REVERSE ===
  Serial.println("Direction: REVERSE, Speed: 2%");
  digitalWrite(directionPin, LOW);   // Reverse
  delay(100);
  digitalWrite(enablePin, HIGH);
  analogWrite(pwmPin, lowSpeedPWM);
  delay(5000);                       // Let it spin reverse

  // === STOP ===
  // Serial.println("Stopping motor");
  // analogWrite(pwmPin, 0);
  // digitalWrite(enablePin, LOW);
  // delay(3000);
}
