#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_SensorLab.h>
#include <Adafruit_AHRS.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Wire.h>

// Fusion filter choice (use only one)
Adafruit_NXPSensorFusion filter;  // NXP Sensor Fusion algorithm

#define FILTER_UPDATE_RATE_HZ 200
uint32_t timestamp = 0;

// Sensor objects
Adafruit_LSM6DSOX sox;
Adafruit_LIS3MDL lis3mdl;
Adafruit_Sensor_Calibration_EEPROM cal;  // EEPROM calibration helper

void recalibrateMag() {
  Serial.println("Recalibrating! Please move the board in all directions for 10 seconds...");
  unsigned long start = millis();
  while (millis() - start < 10000) { // 10-second calibration
    sensors_event_t mag_event;
    lis3mdl.getEvent(&mag_event);
    cal.calibrate(mag_event); // gather samples
    delay(10);
  }
  cal.saveCalibration();
  Serial.println("Calibration complete and saved!");
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println(F("LSM6DSOX + LIS3MDL - NXP Sensor Fusion Example"));

  if (!sox.begin_I2C(0x6A, &Wire1)) {
    Serial.println("Failed to find LSM6DSOX chip");
    while (1) delay(10);
  }
  if (!lis3mdl.begin_I2C(0x1C, &Wire1)) {
    Serial.println("Failed to find LIS3MDL chip");
    while (1) delay(10);
  }

  sox.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  sox.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);

  filter.begin(FILTER_UPDATE_RATE_HZ);
  timestamp = millis();
  Wire1.setClock(400000); // 400KHz for faster I2C

  // Calibration setup
  if (!cal.begin()) {
    Serial.println("Failed to initialize calibration helper");
  } else if (!cal.loadCalibration()) {
    recalibrateMag();
  } else {
    Serial.println("Calibration data loaded from EEPROM.");
  }
}

void loop() {
  // Check for recalibration command
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'c' || command == 'C') {
      recalibrateMag();
    }
  }

  if ((millis() - timestamp) < (1000 / FILTER_UPDATE_RATE_HZ)) {
    return;
  }
  timestamp = millis();

  // Read sensor events
  sensors_event_t accel, gyro, temp, mag;
  sox.getEvent(&accel, &gyro, &temp);
  lis3mdl.getEvent(&mag);

  // Apply mag calibration!
  cal.calibrate(mag);

  // Convert gyro from rad/s to deg/s (NXP fusion expects deg/s)
  float gx = gyro.gyro.x * SENSORS_RADS_TO_DPS;
  float gy = gyro.gyro.y * SENSORS_RADS_TO_DPS;
  float gz = gyro.gyro.z * SENSORS_RADS_TO_DPS;

  // Update the filter
  filter.update(
    gx, gy, gz,
    accel.acceleration.x, accel.acceleration.y, accel.acceleration.z,
    mag.magnetic.x, mag.magnetic.y, mag.magnetic.z
  );

  // Print raw sensor values
  Serial.print("Raw: ");
  Serial.print(accel.acceleration.x, 4); Serial.print(", ");
  Serial.print(accel.acceleration.y, 4); Serial.print(", ");
  Serial.print(accel.acceleration.z, 4); Serial.print(", ");
  Serial.print(gx, 4); Serial.print(", ");
  Serial.print(gy, 4); Serial.print(", ");
  Serial.print(gz, 4); Serial.print(", ");
  Serial.print(mag.magnetic.x, 4); Serial.print(", ");
  Serial.print(mag.magnetic.y, 4); Serial.print(", ");
  Serial.print(mag.magnetic.z, 4); Serial.println();

  // Get and print orientation (Euler angles)
  float yaw = filter.getYaw();
  if (yaw < 0) yaw += 360.0; // Convert to absolute 0-360 degrees
  float pitch = filter.getPitch();
  if (pitch < 0) pitch += 360.0;
  float roll = filter.getRoll();
  if (roll < 0) roll += 360.0;

  Serial.print("Orientation: ");
  Serial.print("Yaw: "); Serial.print(yaw, 2); Serial.print(" deg, ");
  Serial.print("Pitch: "); Serial.print(pitch, 2); Serial.print(" deg, ");
  Serial.print("Roll: "); Serial.print(roll, 2); Serial.println(" deg");

  Serial.print("Loop time: "); Serial.print(millis() - timestamp); Serial.println(" ms");
}
