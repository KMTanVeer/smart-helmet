#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* ================= OLED DISPLAY ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
unsigned long smsDisplayTime = 0;
bool showingSMSMessage = false;

/* ================= DISPLAY STATE ================= */
int signalStrength = 0;
int lastSignalStrength = -1; // Track last displayed value
bool gpsConnected = false;
bool lastGpsConnected = false; // Track last displayed value
int batteryPercent = 100; // Default value - TODO: Connect to battery sensor for real readings
int lastBatteryPercent = -1; // Track last displayed value
bool displayAvailable = false; // Track if OLED is initialized

/* ================= UTILITIES ================= */

#define CSQ_PREFIX_LENGTH 6  // Length of "+CSQ: " prefix

int querySignalStrength() {
  sim800.println("AT+CSQ");
  char response[100];
  int idx = 0;
  unsigned long start = millis();
  
  while (millis() - start < 2000 && idx < 98) { // Leave room for null terminator
    while (sim800.available() && idx < 98) {
      response[idx++] = sim800.read();
    }
    if (idx > 0 && idx < 99) response[idx] = '\0'; // Safe null terminate
    if (strstr(response, "OK") != NULL) break;
  }
  if (idx < 99) response[idx] = '\0'; // Final null termination within bounds
  
  // Parse response: +CSQ: <rssi>,<ber>
  char* csqPtr = strstr(response, "+CSQ:");
  if (csqPtr != NULL) {
    int rssi = 0;
    int ber = 0;
    if (sscanf(csqPtr + CSQ_PREFIX_LENGTH, "%d,%d", &rssi, &ber) == 2) {
      if (rssi >= 0 && rssi <= 31) {
        return rssi;
      }
    }
  }
  return 0; // No signal or error
}

void drawBatteryIcon(int x, int y, int percent) {
  // Draw battery outline (20x10 pixels)
  display.drawRect(x, y, 18, 10, SSD1306_WHITE);
  display.fillRect(x + 18, y + 3, 2, 4, SSD1306_WHITE); // Battery tip
  
  // Fill battery based on percentage
  int fillWidth = (percent * 14) / 100;
  if (fillWidth > 0) {
    display.fillRect(x + 2, y + 2, fillWidth, 6, SSD1306_WHITE);
  }
  
  // Display percentage text
  display.setCursor(x, y + 12);
  display.setTextSize(1);
  display.print(percent);
  display.print("%");
}

void drawSignalBars(int x, int y, int strength) {
  // Draw 4 signal bars based on strength
  // Bar heights: 4, 6, 8, 10 pixels
  int barHeights[4] = {4, 6, 8, 10};
  int barWidth = 3;
  int barSpacing = 2;
  
  int numBars = 0;
  if (strength >= 21) {
    numBars = 4; // Strong: all 4 bars
  } else if (strength >= 11) {
    numBars = 3; // Medium: 3 bars
  } else if (strength > 0) {
    numBars = 1; // Weak: 1 bar
  }
  
  for (int i = 0; i < 4; i++) {
    int barX = x + i * (barWidth + barSpacing);
    int barY = y + (10 - barHeights[i]);
    if (i < numBars) {
      display.fillRect(barX, barY, barWidth, barHeights[i], SSD1306_WHITE);
    } else {
      display.drawRect(barX, barY, barWidth, barHeights[i], SSD1306_WHITE);
    }
  }
  
  // Display signal text
  display.setCursor(x - 5, y + 12);
  display.setTextSize(1);
  if (strength >= 21) {
    display.print("STRONG");
  } else if (strength >= 11) {
    display.print("MEDIUM");
  } else if (strength > 0) {
    display.print("WEAK");
  } else {
    display.print("NO SIG");
  }
}

void updateOLEDDisplay() {
  if (!displayAvailable) return; // Skip if display not initialized
  
  display.clearDisplay();
  
  if (showingSMSMessage) {
    // Display SMS Sent message prominently
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("SMS SENT");
    display.setTextSize(1);
    display.setCursor(25, 45);
    display.print("SUCCESS!");
  } else {
    // Regular status display
    
    // Battery (left side)
    drawBatteryIcon(5, 5, batteryPercent);
    
    // Network signal (right side)
    drawSignalBars(88, 5, signalStrength);
    
    // GPS status (bottom)
    display.setCursor(10, 50);
    display.setTextSize(1);
    if (gpsConnected) {
      display.print("GPS: Connected");
    } else {
      display.print("GPS: Connecting...");
    }
  }
  
  display.display();
}

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
  
  // Initialize OLED
  displayAvailable = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if(!displayAvailable) {
    Serial.println(F("SSD1306 allocation failed"));
    Serial.println(F("System continuing without display..."));
    // System continues without OLED display
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("Smart Helmet");
    display.println("Initializing...");
    display.display();
    delay(2000);
  }
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();

  calibrateGyro();

  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);

  Serial.println("✅ SMART HELMET SYSTEM READY");
  
  // Initial display update
  updateOLEDDisplay();
}

/* ================= LOOP ================= */

void loop() {

  // Cancel false alarm
  if (digitalRead(CANCEL_BTN) == LOW) {
    crashDetected = false;
    smsSent = false;
    showingSMSMessage = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("❌ ALERT CANCELLED");
    updateOLEDDisplay();
    delay(1000);
  }
  
  // Check if SMS message should be cleared after 2 seconds
  if (showingSMSMessage && (millis() - smsDisplayTime >= 2000)) {
    showingSMSMessage = false;
    updateOLEDDisplay();
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
  
  // Update GPS connection status
  gpsConnected = gps.location.isValid();

  if (crashDetected && gps.location.isValid() && !smsSent) {
    if (sendSMS(gps.location.lat(), gps.location.lng())) {
      smsSent = true;
      // Show SMS sent message on OLED
      showingSMSMessage = true;
      smsDisplayTime = millis();
      updateOLEDDisplay();
    }
  }

  if (crashDetected && !gps.location.isValid()) {
    Serial.println("✖ GPS NOT READY");
  }
  
  // Query signal strength periodically (every 5 seconds)
  static unsigned long lastSignalQuery = 0;
  static bool firstUpdate = true; // Track if we need initial update
  if (millis() - lastSignalQuery >= 5000) {
    signalStrength = querySignalStrength();
    lastSignalQuery = millis();
  }
  
  // TODO: Update batteryPercent from battery sensor reading here
  // Example hardware setup: Connect battery through voltage divider to ADC pin (e.g., GPIO34)
  // Voltage divider: R1=100kΩ, R2=100kΩ to scale 4.2V (full) to 2.1V (within ESP32 3.3V limit)
  // Example code:
  //   #define BATTERY_PIN 34
  //   float voltage = analogRead(BATTERY_PIN) * (3.3 / 4095.0) * 2.0; // Adjust multiplier for divider ratio
  //   batteryPercent = map(voltage * 100, 330, 420, 0, 100); // Map 3.3V-4.2V to 0-100%
  //   batteryPercent = constrain(batteryPercent, 0, 100);
  
  // Update display when state changes or on first loop iteration
  bool stateChanged = (signalStrength != lastSignalStrength) ||
                      (gpsConnected != lastGpsConnected) ||
                      (batteryPercent != lastBatteryPercent) ||
                      firstUpdate;
  
  if (!showingSMSMessage && stateChanged) {
    updateOLEDDisplay();
    // Update tracking variables only if they actually changed
    if (signalStrength != lastSignalStrength) lastSignalStrength = signalStrength;
    if (gpsConnected != lastGpsConnected) lastGpsConnected = gpsConnected;
    if (batteryPercent != lastBatteryPercent) lastBatteryPercent = batteryPercent;
    firstUpdate = false;
  }

  delay(200);
}
