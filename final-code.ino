/* 
 * =====================================================
 * SMART HELMET - ENHANCED VERSION WITH PASSIVE BUZZER
 * =====================================================
 * 
 * Enhanced version with:
 * - Passive buzzer with tones (power-on, warning, beep patterns)
 * - 3 emergency contacts for SMS
 * - Working battery monitoring
 * - Crash data logging with timestamps
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
 *   - Battery+ -> R1(100K) -> GPIO 34 (ADC) -> R2(100K) -> GND
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
 * 1. Change PHONE_NUMBER_1, PHONE_NUMBER_2, PHONE_NUMBER_3
 * 2. Adjust ACC_THRESHOLD and GYRO_THRESHOLD if needed
 * 3. Calibrate battery voltage divider ratio if needed
 * 4. Install required libraries:
 *    - Wire (built-in)
 *    - TinyGPSPlus (by Mikal Hart)
 *    - Adafruit_GFX and Adafruit_SSD1306
 * 
 * Author: Smart Helmet Enhanced Project
 * Version: 2.0
 * Date: 2024
 */

#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <time.h>

/* ================= WIFI CONFIGURATION ================= */
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ CHANGE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ CHANGE THIS

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

// THREE EMERGENCY CONTACTS
const char PHONE_NUMBER_1[] = "+8801758161680";   // ⚠️ CHANGE TO CONTACT 1
const char PHONE_NUMBER_2[] = "+8801747213525";   // ⚠️ CHANGE TO CONTACT 2


/* ================= PASSIVE BUZZER & BUTTON ================= */
#define BUZZER_PIN 25   // Passive buzzer for tones
#define CANCEL_BTN 33   // Push button to cancel false alarms

/* ================= BATTERY MONITORING ================= */
#define BATTERY_PIN 34  // ADC pin for battery voltage reading
// Voltage divider: 100K (R1) + 47K (R2) → ratio ≈ 3.13

// Battery voltage range: 3.3V (empty) to 4.2V (full)
// After divider: 1.65V to 2.1V (within ESP32 ADC range)
#define BATTERY_FULL_VOLTAGE 4.2
#define BATTERY_EMPTY_VOLTAGE 3.3
#define VOLTAGE_DIVIDER_RATIO 3.13

/* ================= THRESHOLDS (CONFIGURABLE VIA WEB) ================= */
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

/* ================= WEB SERVER & WEBSOCKET ================= */
WebServer server(80);
WebSocketsServer webSocket(81);

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

/* ================= CRASH DATA LOGGING ================= */
struct CrashData {
  unsigned long timestamp;  // millis() when crash occurred
  float latitude;
  float longitude;
  float accMagnitude;
  float gyroMagnitude;
  int batteryLevel;
  char dateTime[32];        // Date/time string in BD timezone
};

#define MAX_CRASH_LOGS 50
int crashLogCount = 0;

/* ================= REAL-TIME DATA STORAGE FOR WEB DASHBOARD ================= */
#define MAX_DATA_POINTS 1000
struct DataPoint {
  unsigned long timestamp;
  float accMag;
  float gyroMag;
  bool isCrash;
  char dateTime[32];
};
DataPoint dataHistory[MAX_DATA_POINTS];
int dataIndex = 0;

/* ================= TIMEZONE CONFIGURATION ================= */
const long BD_TIMEZONE_OFFSET = 6 * 3600; // UTC+6 for Dhaka
const char* NTP_SERVER = "pool.ntp.org";

/* ================= UTILITIES ================= */

// Get current date/time string in BD timezone
void getCurrentDateTime(char* buffer, size_t bufSize) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  now += BD_TIMEZONE_OFFSET; // Add BD timezone offset
  gmtime_r(&now, &timeinfo);
  
  strftime(buffer, bufSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
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
  float batteryVoltage = vAdc * 3.13;   // ✅ 100k / 47k divider

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

// Send SMS to all 3 contacts
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
  
  // Get current date/time
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
  crashLogCount++;
  
  Serial.println("✅ Crash data logged");
  Serial.print("Total crashes logged: ");
  Serial.println(crashLogCount);
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

/* ================= WEB SERVER HANDLERS ================= */

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Helmet Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 30px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .subtitle {
            color: #666;
            font-size: 14px;
        }
        .status {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 15px;
            margin-top: 20px;
        }
        .status-item {
            background: #f9fafb;
            padding: 10px 20px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
        }
        .status-ok { color: #22c55e; }
        .status-warn { color: #f59e0b; }
        .status-error { color: #ef4444; }
        .card {
            background: white;
            padding: 25px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        h2 {
            color: #333;
            margin-bottom: 20px;
            font-size: 1.3em;
        }
        .chart-container {
            position: relative;
            height: 300px;
            margin-bottom: 20px;
        }
        .controls {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 15px;
        }
        input, button {
            padding: 10px 15px;
            border-radius: 8px;
            border: 2px solid #667eea;
            font-size: 14px;
        }
        button {
            background: #667eea;
            color: white;
            cursor: pointer;
            font-weight: bold;
            transition: all 0.3s;
        }
        button:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
        }
        button:active {
            transform: translateY(0);
        }
        .crash-list {
            max-height: 400px;
            overflow-y: auto;
        }
        .crash-item {
            background: #fee;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 10px;
            border-left: 4px solid #ef4444;
        }
        .crash-time {
            font-weight: bold;
            color: #ef4444;
        }
        .crash-data {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
            margin: 10px 0;
            font-size: 0.9em;
        }
        @media (max-width: 768px) {
            .chart-container {
                height: 250px;
            }
            .controls {
                flex-direction: column;
            }
            input, button {
                width: 100%;
            }
        }
        .connection-status {
            position: fixed;
            top: 10px;
            right: 10px;
            padding: 8px 15px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
            z-index: 1000;
        }
        .connected {
            background: #22c55e;
            color: white;
        }
        .disconnected {
            background: #ef4444;
            color: white;
        }
        .realtime-values {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        .value-box {
            background: #f9fafb;
            padding: 15px;
            border-radius: 10px;
            text-align: center;
        }
        .value-label {
            font-size: 12px;
            color: #666;
            margin-bottom: 5px;
        }
        .value-number {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
        }
        .crash-marker {
            color: #ef4444;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div id="connectionStatus" class="connection-status disconnected">● DISCONNECTED</div>
    
    <div class="container">
        <div class="header">
            <h1>🛡️ Smart Helmet Dashboard</h1>
            <p class="subtitle">Real-Time Crash Detection with Accelerometer & Gyroscope Monitoring</p>
            <div class="status">
                <div class="status-item">Status: <span id="systemStatus" class="status-ok">Monitoring</span></div>
                <div class="status-item">Crashes Detected: <span id="crashCount" class="status-warn">0</span></div>
                <div class="status-item">Data Points: <span id="dataPoints">0</span></div>
            </div>
        </div>

        <div class="card">
            <h2>📊 Real-Time Values</h2>
            <div class="realtime-values">
                <div class="value-box">
                    <div class="value-label">Acceleration (g)</div>
                    <div class="value-number" id="currentAcc">0.00</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Gyroscope (°/s)</div>
                    <div class="value-number" id="currentGyro">0.00</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Acc Threshold</div>
                    <div class="value-number" id="currentAccThreshold" style="font-size: 20px;">2.50</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Gyro Threshold</div>
                    <div class="value-number" id="currentGyroThreshold" style="font-size: 20px;">120.0</div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>⚙️ MPU Threshold Configuration</h2>
            <div class="controls">
                <input type="number" id="accThreshold" placeholder="Acc Threshold (g)" step="0.1" min="1" max="10" value="2.5">
                <input type="number" id="gyroThreshold" placeholder="Gyro Threshold (°/s)" step="10" min="50" max="300" value="120">
                <input type="number" id="impactTime" placeholder="Impact Time (ms)" step="5" min="10" max="100" value="30">
                <button onclick="updateThresholds()">Update Thresholds</button>
                <button onclick="getCurrentSettings()">Get Current Settings</button>
            </div>
        </div>

        <div class="card">
            <h2>📈 Live Acceleration Graph</h2>
            <div class="chart-container">
                <canvas id="accChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>📈 Live Gyroscope Graph</h2>
            <div class="chart-container">
                <canvas id="gyroChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>🚨 Crash History</h2>
            <div class="controls">
                <button onclick="refreshCrashHistory()">Refresh History</button>
                <button onclick="downloadCSV()">Download CSV</button>
                <button onclick="downloadCrashCSV()">Download Crash CSV</button>
            </div>
            <div id="crashList" class="crash-list">
                <p style="text-align: center; color: #999; padding: 20px;">No crashes detected yet</p>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let accChart, gyroChart;
        const maxDataPoints = 100;
        let accData = [];
        let gyroData = [];
        let timeLabels = [];
        let crashHistory = [];
        let crashMarkers = [];

        // Initialize charts
        function initCharts() {
            const accCtx = document.getElementById('accChart').getContext('2d');
            accChart = new Chart(accCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Acceleration (g)',
                        data: accData,
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        pointRadius: 3,
                        pointHoverRadius: 5,
                        pointBackgroundColor: '#3b82f6'
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Acceleration (g)'
                            }
                        },
                        x: {
                            display: false
                        }
                    },
                    plugins: {
                        legend: {
                            display: true
                        },
                        tooltip: {
                            callbacks: {
                                afterLabel: function(context) {
                                    const idx = context.dataIndex;
                                    if (crashMarkers[idx]) {
                                        return '🚨 CRASH DETECTED';
                                    }
                                    return '';
                                }
                            }
                        }
                    },
                    animation: {
                        duration: 0
                    }
                }
            });

            const gyroCtx = document.getElementById('gyroChart').getContext('2d');
            gyroChart = new Chart(gyroCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Gyroscope (°/s)',
                        data: gyroData,
                        borderColor: '#10b981',
                        backgroundColor: 'rgba(16, 185, 129, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        pointRadius: 3,
                        pointHoverRadius: 5,
                        pointBackgroundColor: '#10b981'
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Gyroscope (°/s)'
                            }
                        },
                        x: {
                            display: false
                        }
                    },
                    plugins: {
                        legend: {
                            display: true
                        },
                        tooltip: {
                            callbacks: {
                                afterLabel: function(context) {
                                    const idx = context.dataIndex;
                                    if (crashMarkers[idx]) {
                                        return '🚨 CRASH DETECTED';
                                    }
                                    return '';
                                }
                            }
                        }
                    },
                    animation: {
                        duration: 0
                    }
                }
            });
        }

        // Initialize WebSocket
        function initWebSocket() {
            const wsUrl = 'ws://' + window.location.hostname + ':81';
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                console.log('WebSocket Connected');
                document.getElementById('connectionStatus').className = 'connection-status connected';
                document.getElementById('connectionStatus').textContent = '● CONNECTED';
            };
            
            ws.onclose = function() {
                console.log('WebSocket Disconnected');
                document.getElementById('connectionStatus').className = 'connection-status disconnected';
                document.getElementById('connectionStatus').textContent = '● DISCONNECTED';
                setTimeout(initWebSocket, 3000);
            };
            
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    handleWebSocketData(data);
                } catch(e) {
                    console.error('Error parsing data:', e);
                }
            };
        }

        // Handle incoming WebSocket data
        function handleWebSocketData(data) {
            if (data.type === 'sensor') {
                updateSensorData(data);
            } else if (data.type === 'crash') {
                handleCrashEvent(data);
            } else if (data.type === 'settings') {
                displaySettings(data);
            }
        }

        // Update sensor data on charts
        function updateSensorData(data) {
            // Update real-time values
            document.getElementById('currentAcc').textContent = data.acc.toFixed(2);
            document.getElementById('currentGyro').textContent = data.gyro.toFixed(2);
            
            // Update charts
            if (accData.length >= maxDataPoints) {
                accData.shift();
                gyroData.shift();
                timeLabels.shift();
                crashMarkers.shift();
            }
            
            accData.push(data.acc);
            gyroData.push(data.gyro);
            timeLabels.push('');
            crashMarkers.push(data.crash || false);
            
            // Update point colors for crashes
            const pointColors = accData.map((val, idx) => crashMarkers[idx] ? '#ef4444' : '#3b82f6');
            const pointSizes = accData.map((val, idx) => crashMarkers[idx] ? 10 : 3);
            const gyroPointColors = gyroData.map((val, idx) => crashMarkers[idx] ? '#ef4444' : '#10b981');
            
            accChart.data.datasets[0].pointBackgroundColor = pointColors;
            accChart.data.datasets[0].pointRadius = pointSizes;
            gyroChart.data.datasets[0].pointBackgroundColor = gyroPointColors;
            gyroChart.data.datasets[0].pointRadius = pointSizes;
            
            // Show crash status
            if (data.crash) {
                document.getElementById('systemStatus').className = 'status-error';
                document.getElementById('systemStatus').textContent = 'CRASH DETECTED!';
                
                setTimeout(() => {
                    document.getElementById('systemStatus').className = 'status-ok';
                    document.getElementById('systemStatus').textContent = 'Monitoring';
                }, 3000);
            }
            
            accChart.update();
            gyroChart.update();
            
            document.getElementById('dataPoints').textContent = accData.length;
        }

        // Handle crash event
        function handleCrashEvent(data) {
            document.getElementById('crashCount').textContent = data.count;
            crashHistory.push(data);
            refreshCrashHistory();
        }

        // Update threshold configuration
        function updateThresholds() {
            const acc = document.getElementById('accThreshold').value;
            const gyro = document.getElementById('gyroThreshold').value;
            const impact = document.getElementById('impactTime').value;
            
            fetch('/updateThresholds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({
                    acc: parseFloat(acc),
                    gyro: parseFloat(gyro),
                    impact: parseInt(impact)
                })
            })
            .then(response => response.json())
            .then(data => {
                alert('Thresholds updated successfully!');
                getCurrentSettings();
            })
            .catch(error => {
                alert('Error updating thresholds: ' + error);
            });
        }

        // Get current settings
        function getCurrentSettings() {
            fetch('/getSettings')
            .then(response => response.json())
            .then(data => {
                displaySettings(data);
            });
        }

        // Display current settings
        function displaySettings(data) {
            document.getElementById('accThreshold').value = data.acc;
            document.getElementById('gyroThreshold').value = data.gyro;
            document.getElementById('impactTime').value = data.impact;
            document.getElementById('currentAccThreshold').textContent = data.acc.toFixed(2);
            document.getElementById('currentGyroThreshold').textContent = data.gyro.toFixed(1);
        }

        // Refresh crash history
        function refreshCrashHistory() {
            fetch('/getCrashHistory')
            .then(response => response.json())
            .then(data => {
                const crashList = document.getElementById('crashList');
                if (data.crashes && data.crashes.length > 0) {
                    crashList.innerHTML = '';
                    data.crashes.forEach(crash => {
                        const crashDiv = document.createElement('div');
                        crashDiv.className = 'crash-item';
                        crashDiv.innerHTML = `
                            <div class="crash-time">🚨 CRASH DETECTED</div>
                            <div class="crash-data">
                                <div><strong>Date/Time (BD):</strong> ${crash.dateTime}</div>
                                <div><strong>Acceleration:</strong> ${crash.acc.toFixed(2)} g</div>
                                <div><strong>Gyroscope:</strong> ${crash.gyro.toFixed(2)} °/s</div>
                            </div>
                        `;
                        crashList.appendChild(crashDiv);
                    });
                    document.getElementById('crashCount').textContent = data.crashes.length;
                } else {
                    crashList.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">No crashes detected yet</p>';
                }
            });
        }

        // Download all data CSV
        function downloadCSV() {
            window.location.href = '/downloadCSV';
        }

        // Download crash data CSV from SPIFFS
        function downloadCrashCSV() {
            window.location.href = '/downloadCrashCSV';
        }

        // Initialize on page load
        window.onload = function() {
            initCharts();
            initWebSocket();
            getCurrentSettings();
            refreshCrashHistory();
            
            setInterval(() => {
                document.getElementById('dataPoints').textContent = accData.length;
            }, 1000);
        };
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleUpdateThresholds() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    ACC_THRESHOLD = doc["acc"];
    GYRO_THRESHOLD = doc["gyro"];
    IMPACT_TIME_MS = doc["impact"];
    
    Serial.println("⚙️ Thresholds updated:");
    Serial.print("ACC: "); Serial.print(ACC_THRESHOLD);
    Serial.print(" | GYRO: "); Serial.print(GYRO_THRESHOLD);
    Serial.print(" | IMPACT: "); Serial.print(IMPACT_TIME_MS); Serial.println("ms");
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void handleGetSettings() {
  StaticJsonDocument<200> doc;
  doc["acc"] = ACC_THRESHOLD;
  doc["gyro"] = GYRO_THRESHOLD;
  doc["impact"] = IMPACT_TIME_MS;
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleGetCrashHistory() {
  StaticJsonDocument<4096> doc;
  JsonArray crashes = doc.createNestedArray("crashes");
  
  // Find all crash data points
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].isCrash && dataHistory[i].timestamp > 0) {
      JsonObject crash = crashes.createNestedObject();
      crash["timestamp"] = dataHistory[i].timestamp;
      crash["dateTime"] = String(dataHistory[i].dateTime);
      crash["acc"] = dataHistory[i].accMag;
      crash["gyro"] = dataHistory[i].gyroMag;
    }
  }
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleDownloadCSV() {
  String csv = "Timestamp (ms),Date/Time (BD),Acceleration (g),Gyroscope (deg/s),Crash Detected\n";
  
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].timestamp > 0) {
      csv += String(dataHistory[i].timestamp) + ",";
      csv += String(dataHistory[i].dateTime) + ",";
      csv += String(dataHistory[i].accMag, 2) + ",";
      csv += String(dataHistory[i].gyroMag, 2) + ",";
      csv += dataHistory[i].isCrash ? "YES" : "NO";
      csv += "\n";
    }
  }
  
  server.sendHeader("Content-Disposition", "attachment; filename=smart_helmet_data.csv");
  server.send(200, "text/csv", csv);
}

void handleDownloadCrashCSV() {
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "No crash data found");
    return;
  }
  
  String csv = "";
  while (file.available()) {
    csv += file.readStringUntil('\n') + "\n";
  }
  file.close();
  
  server.sendHeader("Content-Disposition", "attachment; filename=smart_helmet_crashes.csv");
  server.send(200, "text/csv", csv);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // WebSocket event handler (not actively used in this implementation)
}

/* ================= OLED DISPLAY FUNCTIONS ================= */

// Store real-time data point for web dashboard
void storeDataPoint(float accMag, float gyroMag, bool isCrash) {
  dataHistory[dataIndex].timestamp = millis();
  dataHistory[dataIndex].accMag = accMag;
  dataHistory[dataIndex].gyroMag = gyroMag;
  dataHistory[dataIndex].isCrash = isCrash;
  getCurrentDateTime(dataHistory[dataIndex].dateTime, sizeof(dataHistory[dataIndex].dateTime));
  
  dataIndex = (dataIndex + 1) % MAX_DATA_POINTS;
}

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
  Serial.println("  SMART HELMET ENHANCED SYSTEM v2.0   ");
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

  // Initialize data storage
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    dataHistory[i].timestamp = 0;
  }

  // Connect to WiFi
  Serial.println("🔧 Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 30) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("📱 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("🌐 Open browser to: http://");
    Serial.println(WiFi.localIP());
    
    // Initialize NTP time for BD timezone
    configTime(BD_TIMEZONE_OFFSET, 0, NTP_SERVER);
    Serial.println("⏰ Synchronizing time with NTP...");
    delay(2000);
  } else {
    Serial.println("\n⚠️ WiFi connection failed - web dashboard unavailable");
    Serial.println("System will continue with local features only");
  }
  
  // Setup web server routes (only if WiFi connected)
  if (WiFi.status() == WL_CONNECTED) {
    server.on("/", handleRoot);
    server.on("/updateThresholds", HTTP_POST, handleUpdateThresholds);
    server.on("/getSettings", handleGetSettings);
    server.on("/getCrashHistory", handleGetCrashHistory);
    server.on("/downloadCSV", handleDownloadCSV);
    server.on("/downloadCrashCSV", handleDownloadCrashCSV);
    
    // Start web server
    server.begin();
    Serial.println("✅ Web server started on port 80");
    
    // Start WebSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("✅ WebSocket server started on port 81");
  }

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
  
  // ===== WIFI RECONNECTION CHECK =====
  // Check WiFi connection every 30 seconds and attempt to reconnect if disconnected
  static unsigned long lastWiFiCheck = 0;
  static bool wifiWasConnected = true;
  
  if (millis() - lastWiFiCheck >= 30000) {  // Check every 30 seconds
    if (WiFi.status() != WL_CONNECTED) {
      if (wifiWasConnected) {
        Serial.println("⚠️ WiFi connection lost! Attempting to reconnect...");
        wifiWasConnected = false;
      }
      WiFi.disconnect();
      delay(100);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi reconnected!");
        Serial.print("📱 IP Address: ");
        Serial.println(WiFi.localIP());
        wifiWasConnected = true;
      } else {
        Serial.println("\n❌ WiFi reconnection failed - will retry later");
      }
    } else {
      wifiWasConnected = true;
    }
    lastWiFiCheck = millis();
  }
  
  // ===== HANDLE WEB SERVER & WEBSOCKET =====
  // Only handle if WiFi is connected
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
    webSocket.loop();
  }

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
  static int totalCrashCount = 0;
  
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
        totalCrashCount++;
        
        // Send crash event via WebSocket (only if WiFi connected)
        if (WiFi.status() == WL_CONNECTED) {
          StaticJsonDocument<200> crashDoc;
          crashDoc["type"] = "crash";
          crashDoc["count"] = totalCrashCount;
          crashDoc["acc"] = accMag;
          crashDoc["gyro"] = gyroMag;
          
          char dateTime[32];
          getCurrentDateTime(dateTime, sizeof(dateTime));
          crashDoc["dateTime"] = String(dateTime);
          
          String crashJson;
          serializeJson(crashDoc, crashJson);
          webSocket.broadcastTXT(crashJson);
        }
        
        startWarningTone();  // Start 10-second warning tone
        Serial.println("\n🚨🚨🚨 CRASH CONFIRMED 🚨🚨🚨");
      }
    }
  } else {
    impactStart = 0;
  }
  
  // Store data point for web dashboard
  storeDataPoint(accMag, gyroMag, crashNow);
  
  // Send real-time data via WebSocket (only if WiFi connected)
  static unsigned long lastWebSocketSend = 0;
  if (WiFi.status() == WL_CONNECTED && millis() - lastWebSocketSend >= 200) {  // Send every 200ms
    StaticJsonDocument<200> doc;
    doc["type"] = "sensor";
    doc["acc"] = accMag;
    doc["gyro"] = gyroMag;
    doc["crash"] = crashNow;
    
    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
    lastWebSocketSend = millis();
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
    
    // Send SMS to all 3 contacts
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
