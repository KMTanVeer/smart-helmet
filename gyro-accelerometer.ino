#include <Wire.h>
#include <math.h>

#define MPU_ADDR 0x68

// Thresholds (tuned for bike crash)
#define ACC_THRESHOLD 2     // g-force
#define GYRO_THRESHOLD 80  // deg/sec
#define STILL_TIME 3000       // ms after crash

unsigned long crashTime = 0;
bool crashDetected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  Serial.println("MPU6050 Crash Detection Started");
}

void loop() {
  // Read 14 bytes (Accel + Temp + Gyro)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();
  int16_t AcZ = Wire.read() << 8 | Wire.read();

  Wire.read(); Wire.read(); // Skip temperature

  int16_t GyX = Wire.read() << 8 | Wire.read();
  int16_t GyY = Wire.read() << 8 | Wire.read();
  int16_t GyZ = Wire.read() << 8 | Wire.read();

  // Convert to physical units
  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  float gx = GyX / 131.0;
  float gy = GyY / 131.0;
  float gz = GyZ / 131.0;

  float accMag = sqrt(ax * ax + ay * ay + az * az);
  float gyroMag = sqrt(gx * gx + gy * gy + gz * gz);

  Serial.print("Acc(g): ");
  Serial.print(accMag);
  Serial.print(" | Gyro(dps): ");
  Serial.println(gyroMag);

  // Step 1: Detect impact + rotation
  if (!crashDetected && accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    crashDetected = true;
    crashTime = millis();
    Serial.println("⚠️ CRASH IMPACT DETECTED!");
  }

  // Step 2: Check stillness after crash
  if (crashDetected && millis() - crashTime > STILL_TIME) {
    if (accMag < 1.2 && gyroMag < 20) {
      Serial.println("🚨 CONFIRMED CRASH!");
      Serial.println("➡️ SEND SMS / CALL EMERGENCY");
      crashDetected = false;  // reset after confirm
    } else {
      crashDetected = false;  // false alarm
      Serial.println("❌ False alarm, movement detected");
    }
  }

  delay(200);
}
