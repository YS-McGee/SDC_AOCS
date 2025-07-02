// --- Motor Control Pins (from your working code) ---
// These are the pins you've confirmed your motor driver uses.
const int dirPin   = 4; // Your motor direction pin
const int pwmPin   = 5; // Your motor PWM speed pin
const int brakePin = 6; // Your motor brake pin

// Define motor speed values for analogWrite, based on your Nidec motor's logic:
// 255 = Motor OFF / Minimum Speed (corresponds to user input 0)
// 0   = Full Speed (corresponds to user input 255)
const int PWM_OFF_VALUE = 255; // The analogWrite value that makes the motor stop/go slowest
const int PWM_MAX_SPEED = 0;   // The analogWrite value that makes the motor go fastest

// Define states for direction, mapped to your CubeSat's desired turns.
enum MotorDirection {
  STOPPED,
  RIGHT_TURN,        // Represents a "Right" turn (e.g., clockwise motor rotation of reaction wheel)
  LEFT_TURN          // Represents a "Left" turn (e.g., counter-clockwise motor rotation of reaction wheel)
};

// Global variables to hold the current state of the motor.
MotorDirection currentMotorDirection = STOPPED;
int currentMotorSpeedPWM = PWM_OFF_VALUE; // Initialize motor speed to off (PWM 255)

// --- Encoder-related variables (from your working code) ---
// These are included as they were in your functional code, though they are
// not actively used for control commands in this serial interface sketch.
const int encVcc_Pin = 3; // Pin for encoder VCC (if needed to power encoder via Teensy)
const int encA_Pin = 23;  // Encoder Channel A pin
const int encB_Pin = 22;  // Encoder Channel B pin
volatile long encoderTicks = 0; // Accumulates encoder pulses
volatile int direction = 1;     // Tracks encoder direction (not directly used for motor control here)

// --- Define the stop threshold for brakeAndWait ---
// It's good practice to define this as a constant if used consistently.
const float STOP_THRESHOLD_RPS = 0.2; // From your original brakeAndWait default

// --- FUNCTION PROTOTYPES (FORWARD DECLARATIONS) ---
// These declarations tell the compiler about the functions before they are fully defined.
// This resolves the "not declared in this scope" errors.
void setMotor(MotorDirection dir, int speedPWM);
// Corrected prototype for brakeAndWait to match its definition that expects an argument.
// We remove the "= 0.2" from the prototype if the definition doesn't use it,
// OR, we ensure the definition matches the prototype with the default.
// The safest way for Arduino is usually to define the default in the prototype.
// However, since it's causing issues, let's just make sure we call it with an argument.
void brakeAndWait(float stopThresholdRPS); // Removed default here for stricter matching.
float getRPS(); // Function to get Revolutions Per Second (placeholder in this context)
void updateEncoder(); // Interrupt service routine for the encoder

// --- Your Brake Function Implementation ---
// This function engages the brake, waits, and then releases it.
void brakeAndWait(float stopThresholdRPS_param) { // Parameter name changed slightly to avoid confusion
  // Debugging message
  Serial.print("  [BRAKE] Engaging brake with stopThresholdRPS: ");
  Serial.print(stopThresholdRPS_param); // Show the value received
  Serial.println("...");

  // Set PWM to the "off" value (255) to stop sending a speed command.
  analogWrite(pwmPin, PWM_OFF_VALUE);
  digitalWrite(brakePin, LOW);    // Engage brake (based on your working code, LOW engages)
  delay(100);                     // Allow a short time for the brake to engage mechanically.

  // Using a fixed delay to allow the motor to physically stop, as getRPS() is a placeholder here.
  delay(500); // Wait a bit longer to ensure the motor truly stops. Adjust if needed.

  digitalWrite(brakePin, HIGH);   // Release brake (based on your working code, HIGH releases)
  delay(100);                     // Allow a short time for the brake to release.

  // Debugging message
  Serial.println("  [BRAKE] Brake released.");
}

// --- Placeholder getRPS function ---
float getRPS() {
  return 0.0; // Placeholder: returns 0 as encoder is not actively polled in this loop.
}

// --- Arduino setup function ---
void setup() {
  // Configure motor control pins as outputs.
  pinMode(dirPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(brakePin, OUTPUT);

  // Configure encoder pins and power supply for the encoder.
  pinMode(encA_Pin, INPUT);
  pinMode(encB_Pin, INPUT);
  pinMode(encVcc_Pin, OUTPUT);
  digitalWrite(encVcc_Pin, HIGH); // Power the encoder via this pin, as per your old code.

  // Attach the interrupt service routine (ISR) for the encoder.
  attachInterrupt(digitalPinToInterrupt(encA_Pin), updateEncoder, RISING);

  // Initialize serial communication. Using 115200 baud for faster debugging.
  Serial.begin(115200);
  while (!Serial && millis() < 5000) {
    // Wait for serial port to connect. (Useful for Teensy's native USB serial).
  }

  // Print initial instructions and debugging headers.
  Serial.println("\n--- Teensy Motor Control Ready for Yaw Control with Speed Input! ---");
  Serial.println("Commands:");
  Serial.println("  R<speed> : Rotate Right (e.g., R150)");
  Serial.println("  L<speed> : Rotate Left (e.g., L150)");
  Serial.println("  S        : Stop");
  Serial.println("Speed should be an integer between 0 (off/min speed) and 255 (max speed).");
  Serial.println("Note: Internally, 255 PWM is min speed, 0 PWM is max speed for your Nidec.");
  Serial.println("--------------------------------------------------------------------");
  Serial.println("--- Debugging Output Below ---");

  // Initial motor state from your old setup(): engage brake, wait, release.
  analogWrite(pwmPin, PWM_OFF_VALUE); // Set PWM to off state (255).
  digitalWrite(brakePin, LOW);        // Engage brake.
  delay(1500);                        // Wait for 1.5 seconds.
  digitalWrite(brakePin, HIGH);       // Release brake.

  // Ensure motor is fully stopped initially using our dedicated function.
  setMotor(STOPPED, PWM_OFF_VALUE);
}

// --- Arduino loop function ---
void loop() {
  // Check if data is available from the serial port.
  if (Serial.available() > 0) {
    // Read the first character, which should be the command.
    char commandChar = Serial.read();

    // Debugging: Show the raw command character received.
    Serial.print("\nRaw commandChar received: '");
    Serial.print(commandChar);
    Serial.println("'");

    int desiredSpeedInput = -1; // Variable to store user's desired speed (0-255).

    // If the command is 'R' (Right) or 'L' (Left), expect a speed value.
    if (commandChar == 'R' || commandChar == 'L') {
      // Read the rest of the line until a newline character to get the speed number string.
      String speedString = Serial.readStringUntil('\n');
      speedString.trim(); // Remove any leading/trailing whitespace (like carriage return).

      // Debugging: Show the raw speed string received.
      Serial.print("Raw speedString received: \"");
      Serial.print(speedString);
      Serial.println("\"");

      if (speedString.length() > 0) {
        desiredSpeedInput = speedString.toInt(); // Convert string to integer.
        // Clamp the user's input within the valid 0-255 range.
        desiredSpeedInput = constrain(desiredSpeedInput, 0, 255);
        // Debugging: Show the parsed and constrained user input.
        Serial.print("Parsed User Speed Input (0=off, 255=max): ");
        Serial.println(desiredSpeedInput);
      } else {
        // If no speed is provided, default to max speed (user's perspective).
        Serial.println("No speed provided. Using default MAX speed for user input (255).");
        desiredSpeedInput = 255; // Default to user's max speed (corresponds to PWM 0).
      }

      // --- CRITICAL PWM VALUE INVERSION ---
      // Convert the user's intuitive speed (0=off, 255=max) to the motor's actual PWM value (255=min, 0=max).
      currentMotorSpeedPWM = PWM_OFF_VALUE - desiredSpeedInput;
      // Ensure the inverted PWM value is also within the valid 0-255 range.
      currentMotorSpeedPWM = constrain(currentMotorSpeedPWM, PWM_MAX_SPEED, PWM_OFF_VALUE);

    } else if (commandChar == 'S') {
      // If the command is 'S' (Stop), set user's desired speed to 0.
      desiredSpeedInput = 0; // User wants speed 0 (off).
      currentMotorSpeedPWM = PWM_OFF_VALUE; // Actual PWM value for off.
    }

    // Process the command character using a switch statement.
    switch (commandChar) {
      case 'R': // Right turn command.
        Serial.print("Initiating Right Turn at speed (User Input): ");
        Serial.print(desiredSpeedInput);
        Serial.print(" -> Actual PWM value sent to driver: ");
        Serial.println(currentMotorSpeedPWM);
        currentMotorDirection = RIGHT_TURN;
        break; // IMPORTANT: Don't forget break for each case!

      case 'L': // Left turn command.
        Serial.print("Initiating Left Turn at speed (User Input): ");
        Serial.print(desiredSpeedInput);
        Serial.print(" -> Actual PWM value sent to driver: ");
        Serial.println(currentMotorSpeedPWM);
        currentMotorDirection = LEFT_TURN;
        break;

      case 'S': // Stop command.
        Serial.println("Stopping Motor");
        currentMotorDirection = STOPPED;
        currentMotorSpeedPWM = PWM_OFF_VALUE; // Explicitly set to the off PWM value.
        break;

      default: // Handle unknown or unexpected commands.
        Serial.print("Unknown command or parsing error: '");
        Serial.print(commandChar);
        Serial.println("'");
        // Ensure motor is stopped if an invalid command is received.
        currentMotorDirection = STOPPED;
        currentMotorSpeedPWM = PWM_OFF_VALUE;
        break; // Ensure to break from the default case too.
    } // End of switch statement.

    // Debugging: Print the final state variables before calling setMotor.
    Serial.print("Final currentMotorDirection (0=STOP, 1=RIGHT, 2=LEFT): ");
    Serial.println(currentMotorDirection);
    Serial.print("Final currentMotorSpeedPWM: ");
    Serial.println(currentMotorSpeedPWM);

    // Call the setMotor function to apply the new state.
    setMotor(currentMotorDirection, currentMotorSpeedPWM);
  }
  delay(10); // Small delay to prevent overwhelming the serial buffer and for stability.
}

// --- Function to control the motor based on your existing hardware logic ---
void setMotor(MotorDirection dir, int speedPWM) {
  // Debugging: Print the parameters received by setMotor.
  Serial.print("setMotor called with Direction: ");
  Serial.print(dir);
  Serial.print(", Speed PWM Value: ");
  Serial.println(speedPWM);

  // If the command is to STOP, use your `brakeAndWait` function.
  if (dir == STOPPED) {
    // FIX: Pass the default argument explicitly to brakeAndWait()
    brakeAndWait(STOP_THRESHOLD_RPS); // This function handles engaging and releasing the brake.
    // After brakeAndWait, the motor should be stopped and the brake released.
    // Ensure PWM is at the OFF_VALUE in case brakeAndWait changed it temporarily.
    analogWrite(pwmPin, PWM_OFF_VALUE);
    Serial.println("  Motor STOPPED via brakeAndWait and PWM set to OFF_VALUE.");
    return; // Exit the function as stopping sequence is complete.
  }

  // If the motor is commanded to move (RIGHT_TURN or LEFT_TURN):
  // 1. Release the brake.
  digitalWrite(brakePin, HIGH); // Release brake (assuming HIGH releases based on your old code).
  Serial.println("  Brake Released (HIGH).");

  // 2. Set the direction.
  if (dir == RIGHT_TURN) {
    digitalWrite(dirPin, HIGH); // Set direction pin HIGH for Right Turn.
    Serial.println("  dirPin set HIGH (Right Turn).");
  } else { // dir == LEFT_TURN
    digitalWrite(dirPin, LOW);  // Set direction pin LOW for Left Turn.
    Serial.println("  dirPin set LOW (Left Turn).");
  }

  // 3. Apply the PWM speed.
  analogWrite(pwmPin, speedPWM);
  // Debugging: Confirm the analogWrite call.
  Serial.print("  analogWrite(pwmPin, ");
  Serial.print(speedPWM);
  Serial.println(") called.");
}

// --- Your Encoder Interrupt Service Routine (ISR) ---
// This function runs automatically when the encoder's A channel detects a RISING edge.
// It updates the `encoderTicks` count for tracking motor rotation.
void updateEncoder() {
  int B = digitalRead(encB_Pin); // Read the state of encoder Channel B.
  if (B == LOW) {
    encoderTicks++; // If B is LOW, it's one direction.
    direction = 1;
  } else {
    encoderTicks--; // If B is HIGH, it's the other direction.
    direction = -1;
  }
}
