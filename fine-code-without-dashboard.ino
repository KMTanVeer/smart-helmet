/* 
 * =====================================================
 * SMART HELMET - ENHANCED VERSION (WITHOUT WEB DASHBOARD)
 * =====================================================
 * 
 * Enhanced version with:
 * - Passive buzzer with tones (power-on, warning, beep patterns)
 * - 2 emergency contacts for SMS
 * - Working battery monitoring
 * - Crash data logging with timestamps
 * - NO WEB DASHBOARD (WiFi/WebServer removed)
 * 
 * HARDWARE COMPONENTS:
 * --------------------
 * 1. ESP32 Development Board (Main Controller)
 * 2. MPU6050 - 6-axis Gyroscope & Accelerometer
 * 3. Ublox Neo-8M GPS Module
 * 4. SIM800L GSM/GPRS Module
 * 5. PASSIVE Buzzer (Tone Generator)
 * 6. Push Button (Cancel False Alarms)
 * 7. OLED Display SSD1306 (128x64)
 * 8. Battery Voltage Sensor (Voltage Divider)
 * 
 * WIRING CONNECTIONS:
 * -------------------
 * MPU6050:
 *   - VCC  -> 3.3V
 *   - GND  -> GND
 *   - SDA  -> GPIO 21
 *   - SCL  -> GPIO 22
 * 
 * GPS (Ublox Neo-8M):
 *   - VCC  -> 5V
 *   - GND  -> GND
 *   - TX   -> GPIO 16 (ESP32 RX2)
 *   - RX   -> GPIO 17 (ESP32 TX2)
 * 
 * SIM800L:
 *   - VCC  -> 4.2V (use buck converter from battery)
 *   - GND  -> GND (common ground with ESP32)
 *   - TX   -> GPIO 26 (ESP32 RX1)
 *   - RX   -> GPIO 27 (ESP32 TX1)
 * 
 * Passive Buzzer:
 *   - Positive -> GPIO 25
 *   - Negative -> GND
 * 
 * Cancel Button:
 *   - One side -> GPIO 33
 *   - Other side -> GND (uses internal pullup)
 * 
 * Battery Voltage Sensor:
 *   - Battery+ -> R1(100K) -> GPIO 34 (ADC) -> R2(47K) -> GND
 *   - This creates voltage divider to read battery voltage safely
 * 
 * OLED Display (SSD1306):
 *   - VCC  -> 3.3V
 *   - GND  -> GND
 *   - SDA  -> GPIO 21 (shared with MPU6050)
 *   - SCL  -> GPIO 22 (shared with MPU6050)
 * 
 * CONFIGURATION:
 * --------------
 * Before uploading:
 * 1. Change PHONE_NUMBER_1, PHONE_NUMBER_2
 * 2. Adjust ACC_THRESHOLD and GYRO_THRESHOLD if needed
 * 3. Calibrate battery voltage divider ratio if needed
 * 4. Install required libraries:
 *    - Wire (built-in)
 *    - TinyGPSPlus (by Mikal Hart)
 *    - Adafruit_GFX and Adafruit_SSD1306
 * 
 * Author: Smart Helmet Enhanced Project
 * Version: 2.0 (No Dashboard)
 * Date: 2024
 */

#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>

/* ================= OLED DISPLAY ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= MPU6050 ================= */
#define MPU_ADDR 0x68  // I2C address of MPU6050
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;  // Raw sensor values
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;  // Calibration offsets

/* ================= GPS ================= */
TinyGPSPlus gps;  // GPS parser object
HardwareSerial gpsSerial(2);  // Use UART2 for GPS communication
#define GPS_RX 16  // ESP32 RX pin connected to GPS TX
#define GPS_TX 17  // ESP32 TX pin connected to GPS RX

/* ================= SIM800L ================= */
HardwareSerial sim800(1);  // Use UART1 for SIM800L communication
#define SIM_RX 26  // ESP32 RX pin connected to SIM800L TX
#define SIM_TX 27  // ESP32 TX pin connected to SIM800L RX

// TWO EMERGENCY CONTACTS
const char PHONE_NUMBER_1[] = "+1234567890";   // ⚠️ CHANGE TO CONTACT 1 (REQUIRED)
const char PHONE_NUMBER_2[] = "+1234567891";   // ⚠️ CHANGE TO CONTACT 2 (REQUIRED)


/* ================= PASSIVE BUZZER & BUTTON ================= */
#define BUZZER_PIN 25   // Passive buzzer for tones
#define CANCEL_BTN 33   // Push button to cancel false alarms

/* ================= BATTERY MONITORING ================= */
#define BATTERY_PIN 34  // ADC pin for battery voltage reading
// Voltage divider: 100K (R1) + 47K (R2) → ratio = (100K + 47K) / 47K ≈ 3.13

// Battery voltage range: 3.3V (empty) to 4.2V (full)
// After divider: 1.65V to 2.1V (within ESP32 ADC range)
#define BATTERY_FULL_VOLTAGE 4.2
#define BATTERY_EMPTY_VOLTAGE 3.3
#define VOLTAGE_DIVIDER_RATIO 3.13

/* ================= THRESHOLDS (CONFIGURABLE) ================= */
float ACC_THRESHOLD = 2.5;      // g-force
float GYRO_THRESHOLD = 120.0;   // degrees per second
int IMPACT_TIME_MS = 30;        // milliseconds

/* ================= TONE DEFINITIONS ================= */
// Musical notes for passive buzzer
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

// Buzzer tone patterns
#define TONE_POWER_ON    NOTE_C5
#define TONE_WARNING     NOTE_A5
#define TONE_BEEP        NOTE_E5

/* ================= STATE VARIABLES ================= */
bool crashDetected = false;
bool smsSent = false;
unsigned long impactStart = 0;
unsigned long crashTime = 0;  // When crash was detected
bool warningTonePlaying = false;
unsigned long warningToneStart = 0;
bool beepTonePlaying = false;
unsigned long beepToneStart = 0;
int beepCount = 0;

/* ================= OLED DISPLAY STATE ================= */
int signalStrength = 0;
int lastSignalStrength = -1;
bool gpsConnected = false;
bool lastGpsConnected = false;
int batteryPercent = 100;
int lastBatteryPercent = -1;
bool displayAvailable = false;
bool showingSMSMessage = false;
unsigned long smsDisplayTime = 0;

/* ================= UTILITIES ================= */

// Get current timestamp string (using millis)
void getCurrentDateTime(char* buffer, size_t bufSize) {
  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  unsigned long days = hours / 24;
  
  seconds %= 60;
  minutes %= 60;
  hours %= 24;
  
  snprintf(buffer, bufSize, "Day%lu %02lu:%02lu:%02lu", days, hours, minutes, seconds);
}

// Reads response from SIM800L with timeout
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

// Checks if SIM800L is ready and registered on network
bool sim800Ready() {
  sim800.println("AT");
  delay(100);
  if (readSIMResponse(1000).indexOf("OK") == -1) {
    Serial.println("❌ SIM800L not responding to AT");
    return false;
  }

  sim800.println("AT+CREG?");
  delay(100);
  String r = readSIMResponse(2000);
  if (r.indexOf(",1") == -1 && r.indexOf(",5") == -1) {
    Serial.println("❌ SIM800L not registered on network");
    Serial.println(r);
    return false;
  }

  sim800.println("AT+CSQ");
  delay(100);
  String csq = readSIMResponse(1000);
  Serial.print("📶 Signal: ");
  Serial.println(csq);

  return true;
}

/* ================= PASSIVE BUZZER FUNCTIONS ================= */

// Power-on notification tone
void playPowerOnTone() {
  Serial.println("🔊 Playing power-on tone");
  tone(BUZZER_PIN, NOTE_C5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_E5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_G5, 300);
  delay(350);
  noTone(BUZZER_PIN);
}

// Start warning tone (continuous for 10 seconds)
void startWarningTone() {
  warningTonePlaying = true;
  warningToneStart = millis();
  tone(BUZZER_PIN, TONE_WARNING);  // Continuous tone
  Serial.println("🚨 Warning tone started");
}

// Stop warning tone and start beep pattern
void stopWarningToneStartBeep() {
  warningTonePlaying = false;
  noTone(BUZZER_PIN);
  beepTonePlaying = true;
  beepToneStart = millis();
  beepCount = 0;
  Serial.println("🔊 Beep pattern started");
}

// Handle beep pattern (beep beep beep for 3 seconds)
void handleBeepPattern() {
  if (!beepTonePlaying) return;
  
  unsigned long elapsed = millis() - beepToneStart;
  
  // Stop after 3 seconds
  if (elapsed >= 3000) {
    beepTonePlaying = false;
    noTone(BUZZER_PIN);
    Serial.println("🔇 Beep pattern completed");
    return;
  }
  
  // Beep pattern: 200ms on, 200ms off
  unsigned long cycleTime = elapsed % 400;
  if (cycleTime < 200) {
    tone(BUZZER_PIN, TONE_BEEP);
  } else {
    noTone(BUZZER_PIN);
  }
}

// Stop all buzzer sounds
void stopAllBuzzerSounds() {
  warningTonePlaying = false;
  beepTonePlaying = false;
  noTone(BUZZER_PIN);
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
  
  Wire.read(); Wire.read(); // Temperature
  
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  Serial.println("🟡 Calibrating gyro (keep helmet still)...");
  delay(2000);

  long sx=0, sy=0, sz=0;
  
  for(int i=0; i<500; i++){
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
  Serial.print("Bias X: "); Serial.print(gyroBiasX);
  Serial.print(" Y: "); Serial.print(gyroBiasY);
  Serial.print(" Z: "); Serial.println(gyroBiasZ);
}

/* ================= BATTERY MONITORING ================= */

// Read battery voltage and calculate percentage
int readBatteryPercent() {
  static int smoothPercent = -1;

  // Average ADC
  float sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(BATTERY_PIN);
    delay(3);
  }

  float adc = sum / 20.0;
  float vAdc = (adc / 4095.0) * 3.3;
  float batteryVoltage = vAdc * VOLTAGE_DIVIDER_RATIO;   // ✅ Use defined constant

  int rawPercent = (int)((batteryVoltage - BATTERY_EMPTY_VOLTAGE) * 100.0 /
                         (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE));

  rawPercent = constrain(rawPercent, 0, 100);

  // ✅ Startup-safe smoothing
  if (smoothPercent < 0) {
    smoothPercent = rawPercent;   // initialize once
  } else {
    smoothPercent = (smoothPercent * 7 + rawPercent) / 8;
  }

  return smoothPercent;
}


/* ================= SMS FUNCTIONS ================= */

// Send SMS to a specific number
bool sendSMSToNumber(float lat, float lon, const char* phoneNumber) {
  String msg = "🚨 ACCIDENT DETECTED!\n";
  msg += "Emergency alert from Smart Helmet.\n\n";
  msg += "Location Details:\n";
  msg += "Latitude: " + String(lat, 6) + "\n";
  msg += "Longitude: " + String(lon, 6) + "\n\n";
  msg += "View on Google Maps:\n";
  msg += "https://maps.google.com/?q=";
  msg += String(lat, 6) + "," + String(lon, 6);

  Serial.print("📤 Sending SMS to: ");
  Serial.println(phoneNumber);

  sim800.println("AT+CMGF=1");
  delay(100);
  String resp1 = readSIMResponse(1000);
  if (resp1.indexOf("OK") == -1) {
    Serial.println("❌ Failed to set text mode");
    return false;
  }

  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(500);

  String prompt = readSIMResponse(2000);
  if (prompt.indexOf(">") == -1) {
    Serial.println("❌ SMS PROMPT FAILED");
    return false;
  }

  sim800.print(msg);
  delay(100);
  sim800.write(26); // CTRL+Z

  String result = readSIMResponse(10000);
  if (result.indexOf("+CMGS:") != -1 || result.indexOf("OK") != -1) {
    Serial.print("📨 SMS SENT to: ");
    Serial.println(phoneNumber);
    return true;
  }

  Serial.print("❌ SMS FAILED to: ");
  Serial.println(phoneNumber);
  return false;
}

// Send SMS to all 2 contacts
bool sendSMSToAllContacts(float lat, float lon) {
  if (!sim800Ready()) {
    Serial.println("❌ SIM800L NOT READY");
    return false;
  }

  int successCount = 0;
  
  // Send to contact 1
  if (sendSMSToNumber(lat, lon, PHONE_NUMBER_1)) {
    successCount++;
  }
  delay(2000);  // Delay between messages
  
  // Send to contact 2
  if (sendSMSToNumber(lat, lon, PHONE_NUMBER_2)) {
    successCount++;
  }
  
  Serial.print("✅ SMS sent to ");
  Serial.print(successCount);
  Serial.println(" out of 2 contacts");
  
  return successCount > 0;  // Success if at least one SMS sent
}

/* ================= CRASH DATA LOGGING ================= */

// Initialize SPIFFS file system
void initCrashLogging() {
  Serial.println("🔧 Initializing crash data logging...");
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    return;
  }
  
  // Check if crash log file exists
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    Serial.println("📝 Creating new crash log file");
    file = SPIFFS.open("/crashes.csv", FILE_WRITE);
    if (file) {
      // Write CSV header
      file.println("Timestamp,DateTime,Latitude,Longitude,AccMag,GyroMag,Battery%");
      file.close();
    }
  } else {
    Serial.println("✅ Crash log file exists");
    file.close();
  }
  Serial.println("✅ Crash logging initialized");
}

// Log crash data to SPIFFS
void logCrashData(float lat, float lon, float accMag, float gyroMag, int battery) {
  Serial.println("💾 Logging crash data...");
  
  File file = SPIFFS.open("/crashes.csv", FILE_APPEND);
  if (!file) {
    Serial.println("❌ Failed to open crash log file");
    return;
  }
  
  // Get current timestamp
  char dateTime[32];
  getCurrentDateTime(dateTime, sizeof(dateTime));
  
  // Write crash data as CSV
  file.print(millis());
  file.print(",");
  file.print(dateTime);
  file.print(",");
  file.print(lat, 6);
  file.print(",");
  file.print(lon, 6);
  file.print(",");
  file.print(accMag, 2);
  file.print(",");
  file.print(gyroMag, 2);
  file.print(",");
  file.println(battery);
  
  file.close();
  
  Serial.println("✅ Crash data logged");
}

// Read and display crash logs (for debugging)
void displayCrashLogs() {
  Serial.println("\n========== CRASH LOGS ==========");
  
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    Serial.println("❌ No crash log file found");
    return;
  }
  
  while (file.available()) {
    Serial.println(file.readStringUntil('\n'));
  }
  
  file.close();
  Serial.println("================================\n");
}

/* ================= OLED DISPLAY FUNCTIONS ================= */

int querySignalStrength() {
  sim800.println("AT+CSQ");
  delay(100);
  
  char buffer[64] = {0};
  unsigned long start = millis();
  int idx = 0;
  
  while (millis() - start < 2000 && idx < 63) {
    if (sim800.available()) {
      buffer[idx++] = sim800.read();
    }
  }
  buffer[idx] = '\0';
  
  int rssi = 0;
  const int CSQ_PREFIX_LENGTH = 6;
  char* csqPos = strstr(buffer, "+CSQ:");
  if (csqPos != NULL) {
    sscanf(csqPos + CSQ_PREFIX_LENGTH, "%d", &rssi);
    return rssi;
  }
  
  return 0;
}

void drawBatteryIcon(int x, int y, int percent) {
  const int BATTERY_WIDTH = 18;
  const int BATTERY_HEIGHT = 10;
  const int BATTERY_BORDER = 2;
  
  display.drawRect(x, y, BATTERY_WIDTH, BATTERY_HEIGHT, SSD1306_WHITE);
  display.fillRect(x + BATTERY_WIDTH, y + 3, 2, 4, SSD1306_WHITE);
  
  int maxFillWidth = BATTERY_WIDTH - (2 * BATTERY_BORDER);
  int fillWidth = (percent * maxFillWidth) / 100;
  if (fillWidth > 0) {
    display.fillRect(x + BATTERY_BORDER, y + BATTERY_BORDER, fillWidth, BATTERY_HEIGHT - (2 * BATTERY_BORDER), SSD1306_WHITE);
  }
  
  display.setCursor(x, y + 12);
  display.setTextSize(1);
  display.print(percent);
  display.print("%");
}

void drawSignalBars(int x, int y, int strength) {
  int numBars = 0;
  const char* label = "NO SIG";
  
  if (strength >= 21) {
    numBars = 4;
    label = "STRONG";
  } else if (strength >= 11) {
    numBars = 3;
    label = "MEDIUM";
  } else if (strength > 0) {
    numBars = 1;
    label = "WEAK";
  }
  
  int barHeights[] = {4, 6, 8, 10};
  for (int i = 0; i < 4; i++) {
    int barX = x + (i * 5);
    int barY = y + 10 - barHeights[i];
    
    if (i < numBars) {
      display.fillRect(barX, barY, 3, barHeights[i], SSD1306_WHITE);
    } else {
      display.drawRect(barX, barY, 3, barHeights[i], SSD1306_WHITE);
    }
  }
  
  display.setCursor(x, y + 12);
  display.setTextSize(1);
  display.print(label);
}

void updateOLEDDisplay() {
  if (!displayAvailable) return;
  
  display.clearDisplay();
  
  if (showingSMSMessage) {
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("SMS SENT");
    
    display.setTextSize(1);
    display.setCursor(30, 40);
    display.print("SUCCESS!");
  } else {
    drawBatteryIcon(5, 5, batteryPercent);
    drawSignalBars(88, 5, signalStrength);
    
    display.setTextSize(1);
    display.setCursor(10, 50);
    if (gpsConnected) {
      display.print("GPS: Connected");
    } else {
      display.print("GPS: Connecting...");
    }
  }
  
  display.display();
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println(" SMART HELMET SYSTEM v2.0 (NO DASHBOARD)");
  Serial.println("========================================\n");
  //battery smooth start
  analogReadResolution(12);
  analogSetPinAttenuation(BATTERY_PIN, ADC_11db);

  // Configure pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CANCEL_BTN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);
  noTone(BUZZER_PIN);

  // Initialize I2C
  Serial.println("🔧 Initializing I2C and MPU6050...");
  Wire.begin(21, 22);
  
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  delay(100);
  Serial.println("✅ MPU6050 initialized");

  calibrateGyro();

  // Initialize OLED Display
  Serial.println("🔧 Initializing OLED Display...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("⚠️  SSD1306 allocation failed - display disabled");
    displayAvailable = false;
  } else {
    displayAvailable = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Smart Helmet v2");
    display.println("(No Dashboard)");
    display.println("Initializing...");
    display.display();
    Serial.println("✅ OLED Display initialized");
  }

  // Initialize GPS
  Serial.println("🔧 Initializing GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("✅ GPS module initialized");

  // Initialize SIM800L
  Serial.println("🔧 Initializing SIM800L...");
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(3000);
  
  if (sim800Ready()) {
    Serial.println("✅ SIM800L ready and registered");
    // Play power-on notification when network is ready
    playPowerOnTone();
  } else {
    Serial.println("⚠️  SIM800L not ready");
  }

  // Initialize crash data logging
  initCrashLogging();

  Serial.println("\n========================================");
  Serial.println("   SMART HELMET SYSTEM READY          ");
  Serial.println("   Monitoring for accidents...        ");
  Serial.println("========================================\n");
}

/* ================= LOOP ================= */

void loop() {

  // ===== CANCEL BUTTON CHECK =====
  if (digitalRead(CANCEL_BTN) == LOW) {
    crashDetected = false;
    smsSent = false;
    showingSMSMessage = false;
    stopAllBuzzerSounds();
    Serial.println("❌ ALERT CANCELLED BY USER");
    delay(1000);
  }
  
  // Clear SMS message after 2 seconds
  if (showingSMSMessage && (millis() - smsDisplayTime >= 2000)) {
    showingSMSMessage = false;
    updateOLEDDisplay();
  }

  // ===== HANDLE BUZZER PATTERNS =====
  // Warning tone for 10 seconds after crash
  if (warningTonePlaying) {
    if (millis() - warningToneStart >= 10000) {
      stopWarningToneStartBeep();
    }
  }
  
  // Beep pattern during SMS sending
  if (beepTonePlaying) {
    handleBeepPattern();
  }

  // ===== READ SENSOR DATA =====
  readMPU();

  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  float gx = (GyX - gyroBiasX) / 131.0;
  float gy = (GyY - gyroBiasY) / 131.0;
  float gz = (GyZ - gyroBiasZ) / 131.0;

  float accMag = sqrt(ax*ax + ay*ay + az*az);
  float gyroMag = sqrt(gx*gx + gy*gy + gz*gz);

  Serial.print("ACC(g): "); Serial.print(accMag, 2);
  Serial.print(" | GYRO(dps): "); Serial.print(gyroMag, 1);

  // ===== CRASH DETECTION LOGIC =====
  bool crashNow = false;
  
  if (accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    if (impactStart == 0) {
      impactStart = millis();
      Serial.print(" [IMPACT DETECTED]");
    }
    
    if (millis() - impactStart > IMPACT_TIME_MS) {
      if (!crashDetected) {
        crashDetected = true;
        crashNow = true;
        crashTime = millis();
        
        startWarningTone();  // Start 10-second warning tone
        Serial.println("\n🚨🚨🚨 CRASH CONFIRMED 🚨🚨🚨");
      }
    }
  } else {
    impactStart = 0;
  }
  
  Serial.println();

  // ===== GPS DATA PROCESSING =====
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  
  gpsConnected = gps.location.isValid();

  // ===== EMERGENCY SMS TRANSMISSION =====
  if (crashDetected && gps.location.isValid() && !smsSent) {
    Serial.println("📍 GPS fix acquired - sending emergency SMS...");
    
    // Stop warning tone and start beep pattern
    if (warningTonePlaying) {
      stopWarningToneStartBeep();
    }
    
    // Log crash data
    logCrashData(gps.location.lat(), gps.location.lng(), accMag, gyroMag, batteryPercent);
    
    // Send SMS to all 2 contacts
    if (sendSMSToAllContacts(gps.location.lat(), gps.location.lng())) {
      smsSent = true;
      showingSMSMessage = true;
      smsDisplayTime = millis();
      updateOLEDDisplay();
    } else {
      Serial.println("⚠️  SMS failed - will retry...");
      delay(5000);
    }
  }

  // ===== GPS STATUS MONITORING =====
  if (crashDetected && !gps.location.isValid() && !smsSent) {
    Serial.print("⏳ Waiting for GPS fix... ");
    Serial.print("Satellites: "); Serial.println(gps.satellites.value());
  }

  if (gps.location.isValid()) {
    Serial.print("📍 GPS: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" | Sats: ");
    Serial.println(gps.satellites.value());
  }
  
  // ===== QUERY SIGNAL STRENGTH =====
  static unsigned long lastSignalQuery = 0;
  static bool firstUpdate = true;
  if (millis() - lastSignalQuery >= 5000) {
    signalStrength = querySignalStrength();
    lastSignalQuery = millis();
  }
  
  // ===== READ BATTERY LEVEL =====
  static unsigned long lastBatteryRead = 0;
  if (millis() - lastBatteryRead >= 10000) {  // Read every 10 seconds
    batteryPercent = readBatteryPercent();
    lastBatteryRead = millis();
    Serial.print("🔋 Battery: ");
    Serial.print(batteryPercent);
    Serial.println("%");
  }
  
  // ===== UPDATE DISPLAY =====
  bool stateChanged = (signalStrength != lastSignalStrength) ||
                      (gpsConnected != lastGpsConnected) ||
                      (batteryPercent != lastBatteryPercent) ||
                      firstUpdate;
  
  if (!showingSMSMessage && stateChanged) {
    updateOLEDDisplay();
    if (signalStrength != lastSignalStrength) lastSignalStrength = signalStrength;
    if (gpsConnected != lastGpsConnected) lastGpsConnected = gpsConnected;
    if (batteryPercent != lastBatteryPercent) lastBatteryPercent = batteryPercent;
    firstUpdate = false;
  }

  delay(200);
}
