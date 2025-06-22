const int pwmPin = 10;        // Motor PWM control (white wire)
const int enablePin = 9;      // Enable pin (yellow wire)
const int directionPin = 8;   // Direction control (blue wire)
const int encoderA = 2;       // Encoder signal A (brown wire)

volatile unsigned long pulseCount = 0;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Motor pin setup
  pinMode(pwmPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(directionPin, OUTPUT);

  // Encoder pin setup
  pinMode(encoderA, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderA), countPulse, RISING);

  // PWM config
  analogWriteResolution(8);
  analogWriteFrequency(pwmPin, 25000);  // 25 kHz

  digitalWrite(directionPin, HIGH);  // Forward
  digitalWrite(enablePin, HIGH);     // Enable motor

  Serial.println("Time(sec)\tPWM\tPulses in 10 sec");
}

void loop() {
  for (int duty = 15; duty <= 255; duty += 10) {
    pulseCount = 0;                      // Reset count
    analogWrite(pwmPin, duty);          // Set motor speed
    unsigned long startTime = millis();

    while (millis() - startTime < 10000) {
      // Just wait 10 seconds while pulses are being counted
    }

    // Print results after 10 seconds
    Serial.print(millis() / 1000);      // Current time in seconds
    Serial.print('\t');
    Serial.print(duty);                 // Current PWM value
    Serial.print('\t');
    Serial.println(pulseCount);         // Pulses in 10 sec
  }

  // Stop motor after last step
  analogWrite(pwmPin, 0);
  digitalWrite(enablePin, LOW);
  Serial.println("Test complete. Motor stopped.");
  while (1);  // Stop loop
}
