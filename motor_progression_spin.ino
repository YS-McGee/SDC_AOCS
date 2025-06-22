const int pwmPin = 10;     // PWM output to motor (white wire)
const int enablePin = 9;   // Enable pin to start the motor (yellow wire)

void setup() {
  // Initialize serial monitor
  Serial.begin(9600);
  while (!Serial); // Wait for Serial Monitor to open (optional)

  Serial.println("Motor Test Program: Starting...");

  // Enable motor
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  Serial.println("Motor enabled (enablePin set HIGH)");

  // Configure PWM
  analogWriteFrequency(pwmPin, 25000);  // 25 kHz for Nidec
  analogWriteResolution(8);             // 8-bit resolution (0–255)
}

void loop() {
  Serial.println("Setting speed: 64 (25%)");
  analogWrite(pwmPin, 64);
  delay(3000);

  Serial.println("Setting speed: 128 (50%)");
  analogWrite(pwmPin, 128);
  delay(3000);

  Serial.println("Setting speed: 200 (78%)");
  analogWrite(pwmPin, 200);
  delay(3000);

  Serial.println("Stopping motor (0 PWM)");
  analogWrite(pwmPin, 0);
  delay(3000);
}
