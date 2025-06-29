const int encVcc_Pin = 3;
const int dirPin     = 4;
const int pwmPin     = 5;
const int brakePin   = 6;

// Encoder pins
const int encA_Pin = 23;
const int encB_Pin = 22;

volatile long encoderTicks = 0;
volatile int direction = 1;

struct RPSMap {
  int pwm;
  float targetRPS;
};

RPSMap rpsTargets[] = {
  {240, 2.2}, {225, 4.7}, {210, 7.3}, {195, 9.7}, {180, 12.2},
  {165, 14.6}, {150, 17.1}, {135, 19.6}, {120, 22.1}, {105, 24.6},
  {90, 27.1}, {75, 29.6}, {60, 32.0}, {45, 34.5}, {30, 37.0}, {15, 39.5}
};

bool hasReachedTargetRPS(float currentRPS, float targetRPS, float tolerance = 0.01) {
  return currentRPS >= (targetRPS - tolerance);
}

float getRPS() {
  long startTicks = encoderTicks;
  delay(100);
  long endTicks = encoderTicks;
  // 100 ticks per revolution assumed; adjust if different!
  return abs(endTicks - startTicks) / 100.0 / 0.1;
}

void brakeAndWait(float stopThresholdRPS = 0.2) {
  analogWrite(pwmPin, 255);        // Stop (min PWM)
  digitalWrite(brakePin, LOW);     // Engage brake
  delay(100);                      // Let brake engage

  // Wait for the motor to stop spinning
  while (true) {
    float rps = getRPS();
    if (rps <= stopThresholdRPS) break;
    delay(50);
  }

  digitalWrite(brakePin, HIGH);    // Release brake
  delay(100);
}


void setup() {
  pinMode(pwmPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(brakePin, OUTPUT);
  pinMode(encA_Pin, INPUT);
  pinMode(encB_Pin, INPUT);

  attachInterrupt(digitalPinToInterrupt(encA_Pin), updateEncoder, RISING);
  Serial.begin(9600);

  analogWrite(pwmPin, 255);
  digitalWrite(brakePin, LOW);
  delay(1500);
  digitalWrite(brakePin, HIGH); // Release brake
}

void loop() {
  for (size_t i = 0; i < sizeof(rpsTargets) / sizeof(RPSMap); i++) {
    int pwm = rpsTargets[i].pwm;
    float targetRPS = rpsTargets[i].targetRPS;

    // CW test
    brakeAndWait(); // Ensure stopped before starting
    encoderTicks = 0;
    digitalWrite(dirPin, HIGH);
    analogWrite(pwmPin, pwm);
    unsigned long timeCW = 0;

    while (true) {
      float rps = getRPS();
      if (hasReachedTargetRPS(rps, targetRPS)) {
        timeCW = millis();
        break;
      }
    }

    // Brake before switching direction
    brakeAndWait();

    // CCW test
    encoderTicks = 0;
    digitalWrite(dirPin, LOW);
    analogWrite(pwmPin, pwm);
    unsigned long timeCCW = 0;

    while (true) {
      float rps = getRPS();
      if (hasReachedTargetRPS(rps, targetRPS)) {
        timeCCW = millis();
        break;
      }
    }

    // Brake at end of step
    brakeAndWait();

    // Print result
    Serial.print("PWM: ");
    Serial.print(pwm);
    Serial.print(" | Target RPS: ");
    Serial.print(targetRPS);
    Serial.print(" | CW time: ");
    Serial.print(timeCW);
    Serial.print(" ms | CCW time: ");
    Serial.print(timeCCW);
    Serial.print(" ms | Δt: ");
    Serial.print(abs((long)timeCCW - (long)timeCW) / 1000.0, 3);
    Serial.println(" s");
  }

  analogWrite(pwmPin, 255);
  Serial.println("Sweep complete.");
  while (true);
}

void updateEncoder() {
  int B = digitalRead(encB_Pin);
  if (B == LOW) {
    encoderTicks++;
    direction = 1;
  } else {
    encoderTicks--;
    direction = -1;
  }
}
