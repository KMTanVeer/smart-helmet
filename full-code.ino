#include <Wire.h>
#include <TinyGPSPlus.h>

/* ================= MPU6050 ================= */
#define MPU_ADDR 0x68
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

/* ================= GPS ================= */
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
#define GPS_RX 16
#define GPS_TX 17

/* ================= SIM800L ================= */
HardwareSerial sim800(1);
#define SIM_RX 26
#define SIM_TX 27
const char PHONE_NUMBER[] = "+8801747213525";   // CHANGE NUMBER

/* ================= BUZZER & BUTTON ================= */
#define BUZZER_PIN 25
#define CANCEL_BTN 33

/* ================= THRESHOLDS ================= */
#define ACC_THRESHOLD   2.5     // g (real crash level)
#define GYRO_THRESHOLD  120.0   // dps
#define IMPACT_TIME_MS  30

bool crashDetected = false;
bool smsSent = false;
unsigned long impactStart = 0;

/* ================= UTILITIES ================= */

String readSIMResponse(unsigned long timeout = 3000) {
  String response = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (sim800.available()) {
      response += char(sim800.read());
    }
  }
  return response;
}

bool sim800Ready() {
  sim800.println("AT");
  if (readSIMResponse().indexOf("OK") == -1) return false;

  sim800.println("AT+CREG?");
  String r = readSIMResponse();
  if (r.indexOf(",1") == -1 && r.indexOf(",5") == -1) return false;

  return true;
}

/* ================= MPU FUNCTIONS ================= */

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // temp ignore
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  Serial.println("🟡 Calibrating gyro (keep still)...");
  delay(2000);

  long sx=0, sy=0, sz=0;
  for(int i=0;i<500;i++){
    readMPU();
    sx += GyX;
    sy += GyY;
    sz += GyZ;
    delay(5);
  }

  gyroBiasX = sx / 500.0;
  gyroBiasY = sy / 500.0;
  gyroBiasZ = sz / 500.0;

  Serial.println("✅ Gyro calibrated");
}

/* ================= SMS FUNCTION ================= */

bool sendSMS(float lat, float lon) {

  if (!sim800Ready()) {
    Serial.println("❌ SIM800L NOT READY / NO NETWORK");
    return false;
  }

  String msg = "🚨 ACCIDENT DETECTED!\n";
  msg += "Latitude: " + String(lat,6) + "\n";
  msg += "Longitude: " + String(lon,6) + "\n";
  msg += "Map: https://maps.google.com/?q=";
  msg += String(lat,6) + "," + String(lon,6);

  sim800.println("AT+CMGF=1");
  readSIMResponse();

  sim800.print("AT+CMGS=\"");
  sim800.print(PHONE_NUMBER);
  sim800.println("\"");

  String prompt = readSIMResponse();
  if (prompt.indexOf(">") == -1) {
    Serial.println("❌ SMS PROMPT FAILED");
    return false;
  }

  sim800.print(msg);
  sim800.write(26); // CTRL+Z

  String result = readSIMResponse(8000);
  if (result.indexOf("+CMGS:") != -1) {
    Serial.println("📨 SMS CONFIRMED SENT");
    return true;
  }

  Serial.println("❌ SMS FAILED");
  Serial.println(result);
  return false;
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CANCEL_BTN, INPUT_PULLUP);

  Wire.begin(21,22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  calibrateGyro();

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  Serial.println("✅ SMART HELMET SYSTEM READY");
}

/* ================= LOOP ================= */

void loop() {

  // Cancel false alarm
  if (digitalRead(CANCEL_BTN) == LOW) {
    crashDetected = false;
    smsSent = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("❌ ALERT CANCELLED");
    delay(1000);
  }

  readMPU();

  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  float gx = (GyX - gyroBiasX) / 131.0;
  float gy = (GyY - gyroBiasY) / 131.0;
  float gz = (GyZ - gyroBiasZ) / 131.0;

  float accMag = sqrt(ax*ax + ay*ay + az*az);
  float gyroMag = sqrt(gx*gx + gy*gy + gz*gz);

  Serial.print("ACC(g): "); Serial.print(accMag,2);
  Serial.print(" | GYRO(dps): "); Serial.println(gyroMag,1);

  if (accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    if (impactStart == 0) impactStart = millis();
    if (millis() - impactStart > IMPACT_TIME_MS) {
      crashDetected = true;
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("⚠ CRASH DETECTED");
    }
  } else {
    impactStart = 0;
  }

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (crashDetected && gps.location.isValid() && !smsSent) {
    if (sendSMS(gps.location.lat(), gps.location.lng())) {
      smsSent = true;
    }
  }

  if (crashDetected && !gps.location.isValid()) {
    Serial.println("✖ GPS NOT READY");
  }

  delay(200);
}
