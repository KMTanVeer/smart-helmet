#include <Wire.h>

#define MPU_ADDR 0x68

// Threshold for crash detection
#define CRASH_THRESHOLD 30000   // adjust if needed

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  Serial.println("MPU6050 Crash Detection Started");
}

void loop() {
  // Read accelerometer data
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);

  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();

  // Calculate acceleration magnitude
  long accelMagnitude = sqrt(
    (long)AcX * AcX +
    (long)AcY * AcY +
    (long)AcZ * AcZ
  );

  Serial.print("Accel Mag: ");
  Serial.println(accelMagnitude);

  // Crash detection
  if (accelMagnitude > CRASH_THRESHOLD) {
    Serial.println("🚨 CRASH DETECTED 🚨");
    delay(3000);   // avoid multiple triggers
  }

  delay(200);
}
