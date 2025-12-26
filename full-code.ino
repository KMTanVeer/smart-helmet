/* 
 * =====================================================
 * SMART HELMET - ACCIDENT DETECTION & EMERGENCY ALERT
 * =====================================================
 * 
 * This system detects motorcycle/bike accidents using MPU6050
 * sensor and automatically sends emergency SMS with GPS location
 * via SIM800L GSM module.
 * 
 * HARDWARE COMPONENTS:
 * --------------------
 * 1. ESP32 Development Board (Main Controller)
 * 2. MPU6050 - 6-axis Gyroscope & Accelerometer
 * 3. Ublox Neo-8M GPS Module
 * 4. SIM800L GSM/GPRS Module
 * 5. Buzzer (Alert)
 * 6. Push Button (Cancel False Alarms)
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
 *   - RST  -> Not connected (optional: GPIO for reset)
 * 
 * Buzzer:
 *   - Positive -> GPIO 25
 *   - Negative -> GND
 * 
 * Cancel Button:
 *   - One side -> GPIO 33
 *   - Other side -> GND (uses internal pullup)
 * 
 * POWER SUPPLY NOTES:
 * -------------------
 * - SIM800L requires 3.4V-4.4V with high current (2A peak)
 * - Use a dedicated power supply or battery with buck converter
 * - ESP32 and sensors can share 3.3V/5V regulation
 * - Ensure common ground for all modules
 * 
 * CONFIGURATION:
 * --------------
 * Before uploading:
 * 1. Change PHONE_NUMBER to your emergency contact
 * 2. Adjust ACC_THRESHOLD and GYRO_THRESHOLD if needed
 * 3. Install required libraries:
 *    - Wire (built-in)
 *    - TinyGPSPlus (by Mikal Hart)
 * 
 * OPERATION:
 * ----------
 * 1. System initializes and calibrates gyroscope
 * 2. Continuously monitors acceleration and rotation
 * 3. Detects crash when both thresholds exceeded for >30ms
 * 4. Activates buzzer and waits for GPS fix
 * 5. Sends SMS with Google Maps location link
 * 6. User can cancel false alarm with button
 * 
 * Author: Smart Helmet Project
 * Version: 1.0
 * Date: 2024
 */

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
const char PHONE_NUMBER[] = "+1234567890";   // ⚠️ CHANGE TO YOUR EMERGENCY CONTACT

/* ================= BUZZER & BUTTON ================= */
#define BUZZER_PIN 25   // Active buzzer for alert
#define CANCEL_BTN 33   // Push button to cancel false alarms

/* ================= THRESHOLDS ================= */
// These values are tuned for motorcycle/bike crash detection
// Adjust based on testing in real conditions
#define ACC_THRESHOLD   2.5     // g-force (1g = normal gravity, 2.5g = significant impact)
#define GYRO_THRESHOLD  120.0   // degrees per second (rapid rotation indicates fall)
#define IMPACT_TIME_MS  30      // milliseconds (sustained impact to filter vibrations)

/* ================= STATE VARIABLES ================= */
bool crashDetected = false;  // Flag: crash has been confirmed
bool smsSent = false;        // Flag: emergency SMS has been sent
unsigned long impactStart = 0;  // Timestamp when impact first detected

/* ================= OLED DISPLAY STATE ================= */
int signalStrength = 0;           // RSSI value (0-31) from SIM800L
int lastSignalStrength = -1;      // Previous signal strength for change detection
bool gpsConnected = false;         // GPS fix status
bool lastGpsConnected = false;     // Previous GPS status for change detection
int batteryPercent = 100;          // Battery level (default 100%)
int lastBatteryPercent = -1;       // Previous battery level for change detection
bool displayAvailable = false;     // Flag: OLED initialized successfully
bool showingSMSMessage = false;    // Flag: SMS notification is being displayed
unsigned long smsDisplayTime = 0;  // Timestamp when SMS message was shown

/* ================= UTILITIES ================= */

// Reads response from SIM800L with timeout
// Returns the complete response as a String
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
// Returns true if module responds and is registered
bool sim800Ready() {
  // Test basic communication
  sim800.println("AT");
  delay(100);
  if (readSIMResponse(1000).indexOf("OK") == -1) {
    Serial.println("❌ SIM800L not responding to AT");
    return false;
  }

  // Check network registration
  sim800.println("AT+CREG?");
  delay(100);
  String r = readSIMResponse(2000);
  // +CREG: 0,1 (registered, home network) or +CREG: 0,5 (registered, roaming)
  if (r.indexOf(",1") == -1 && r.indexOf(",5") == -1) {
    Serial.println("❌ SIM800L not registered on network");
    Serial.println(r);
    return false;
  }

  // Check signal strength
  sim800.println("AT+CSQ");
  delay(100);
  String csq = readSIMResponse(1000);
  Serial.print("📶 Signal: ");
  Serial.println(csq);

  return true;
}

/* ================= MPU FUNCTIONS ================= */

// Reads raw accelerometer and gyroscope data from MPU6050
// Data is stored in global variables AcX, AcY, AcZ, GyX, GyY, GyZ
void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // Starting register for accelerometer data
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);  // Read 14 bytes: 6 accel + 2 temp + 6 gyro

  // Read accelerometer values (3 axes, 16-bit each)
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  
  Wire.read(); Wire.read(); // Temperature - not used
  
  // Read gyroscope values (3 axes, 16-bit each)
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

// Calibrates the gyroscope by computing bias values
// Device must be kept still during calibration
void calibrateGyro() {
  Serial.println("🟡 Calibrating gyro (keep helmet still)...");
  delay(2000);

  long sx=0, sy=0, sz=0;
  
  // Take 500 samples to calculate average bias
  for(int i=0; i<500; i++){
    readMPU();
    sx += GyX;
    sy += GyY;
    sz += GyZ;
    delay(5);
  }

  // Calculate average bias for each axis
  gyroBiasX = sx / 500.0;
  gyroBiasY = sy / 500.0;
  gyroBiasZ = sz / 500.0;

  Serial.println("✅ Gyro calibrated");
  Serial.print("Bias X: "); Serial.print(gyroBiasX);
  Serial.print(" Y: "); Serial.print(gyroBiasY);
  Serial.print(" Z: "); Serial.println(gyroBiasZ);
}

/* ================= SMS FUNCTION ================= */

// Sends emergency SMS with accident location via Google Maps link
// Returns true if SMS was successfully sent, false otherwise
bool sendSMS(float lat, float lon) {

  if (!sim800Ready()) {
    Serial.println("❌ SIM800L NOT READY / NO NETWORK");
    return false;
  }

  // Construct emergency message with location details
  String msg = "🚨 ACCIDENT DETECTED!\n";
  msg += "Emergency alert from Smart Helmet.\n\n";
  msg += "Location Details:\n";
  msg += "Latitude: " + String(lat, 6) + "\n";
  msg += "Longitude: " + String(lon, 6) + "\n\n";
  msg += "View on Google Maps:\n";
  msg += "https://maps.google.com/?q=";
  msg += String(lat, 6) + "," + String(lon, 6);

  Serial.println("📤 Preparing to send SMS...");

  // Set SMS to text mode
  sim800.println("AT+CMGF=1");
  delay(100);
  String resp1 = readSIMResponse(1000);
  if (resp1.indexOf("OK") == -1) {
    Serial.println("❌ Failed to set text mode");
    return false;
  }

  // Send recipient number
  sim800.print("AT+CMGS=\"");
  sim800.print(PHONE_NUMBER);
  sim800.println("\"");
  delay(500);

  // Wait for '>' prompt from SIM800L
  String prompt = readSIMResponse(2000);
  if (prompt.indexOf(">") == -1) {
    Serial.println("❌ SMS PROMPT FAILED - No '>' received");
    Serial.println(prompt);
    return false;
  }

  // Send message content
  sim800.print(msg);
  delay(100);
  sim800.write(26); // Send CTRL+Z to indicate end of message

  // Wait for confirmation
  String result = readSIMResponse(10000);  // SMS can take time
  if (result.indexOf("+CMGS:") != -1 || result.indexOf("OK") != -1) {
    Serial.println("📨 SMS SUCCESSFULLY SENT");
    return true;
  }

  Serial.println("❌ SMS SENDING FAILED");
  Serial.println(result);
  return false;
}

/* ================= OLED DISPLAY FUNCTIONS ================= */

// Queries signal strength from SIM800L using AT+CSQ command
// Returns RSSI value (0-31), or 0 if query fails
int querySignalStrength() {
  sim800.println("AT+CSQ");
  delay(100);
  
  char buffer[64] = {0};
  unsigned long start = millis();
  int idx = 0;
  
  // Read response with timeout
  while (millis() - start < 2000 && idx < 63) {
    if (sim800.available()) {
      buffer[idx++] = sim800.read();
    }
  }
  buffer[idx] = '\0';
  
  // Parse "+CSQ: <rssi>,<ber>" response
  int rssi = 0;
  const int CSQ_PREFIX_LENGTH = 6; // Length of "+CSQ: "
  char* csqPos = strstr(buffer, "+CSQ:");
  if (csqPos != NULL) {
    sscanf(csqPos + CSQ_PREFIX_LENGTH, "%d", &rssi);
    return rssi;
  }
  
  return 0; // No signal or error
}

// Draws battery icon with fill level at specified position
// Parameters: x, y = top-left position, percent = battery level (0-100)
void drawBatteryIcon(int x, int y, int percent) {
  // Draw battery outline (18x10 pixels)
  display.drawRect(x, y, 18, 10, SSD1306_WHITE);
  
  // Draw battery tip (2x4 pixels)
  display.fillRect(x + 18, y + 3, 2, 4, SSD1306_WHITE);
  
  // Calculate fill width based on percentage
  int fillWidth = (percent * 14) / 100; // Max 14 pixels inside (18-2-2)
  if (fillWidth > 0) {
    display.fillRect(x + 2, y + 2, fillWidth, 6, SSD1306_WHITE);
  }
  
  // Display percentage text below icon
  display.setCursor(x, y + 12);
  display.setTextSize(1);
  display.print(percent);
  display.print("%");
}

// Draws signal strength bars at specified position
// Parameters: x, y = top-left position, strength = RSSI value (0-31)
void drawSignalBars(int x, int y, int strength) {
  // Determine number of bars based on signal strength
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
  
  // Draw 4 signal bars with varying heights
  int barHeights[] = {4, 6, 8, 10};
  for (int i = 0; i < 4; i++) {
    int barX = x + (i * 5);
    int barY = y + 10 - barHeights[i];
    
    if (i < numBars) {
      // Filled bar for active signal
      display.fillRect(barX, barY, 3, barHeights[i], SSD1306_WHITE);
    } else {
      // Outlined bar for inactive
      display.drawRect(barX, barY, 3, barHeights[i], SSD1306_WHITE);
    }
  }
  
  // Display signal quality label below bars
  display.setCursor(x, y + 12);
  display.setTextSize(1);
  display.print(label);
}

// Main display update function - refreshes OLED screen
void updateOLEDDisplay() {
  if (!displayAvailable) return;  // Skip if display not initialized
  
  display.clearDisplay();
  
  if (showingSMSMessage) {
    // Display SMS sent notification
    display.setTextSize(2);
    display.setCursor(10, 20);
    display.print("SMS SENT");
    
    display.setTextSize(1);
    display.setCursor(30, 40);
    display.print("SUCCESS!");
  } else {
    // Display regular status screen
    
    // Battery icon (top-left)
    drawBatteryIcon(5, 5, batteryPercent);
    
    // Signal strength bars (top-right)
    drawSignalBars(88, 5, signalStrength);
    
    // GPS connection status (bottom)
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
  // Initialize serial communication for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n========================================");
  Serial.println("   SMART HELMET SYSTEM INITIALIZING   ");
  Serial.println("========================================\n");

  // Configure buzzer and button pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CANCEL_BTN, INPUT_PULLUP);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off initially

  // Initialize I2C for MPU6050 (SDA=21, SCL=22)
  Serial.println("🔧 Initializing MPU6050...");
  Wire.begin(21, 22);
  
  // Wake up MPU6050 (it starts in sleep mode)
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // Power management register
  Wire.write(0);     // Wake up (set sleep bit to 0)
  Wire.endTransmission();
  delay(100);
  Serial.println("✅ MPU6050 initialized");

  // Calibrate gyroscope for accurate readings
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
    display.println("Smart Helmet");
    display.println("Initializing...");
    display.display();
    Serial.println("✅ OLED Display initialized");
  }

  // Initialize GPS module on UART2
  Serial.println("🔧 Initializing GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("✅ GPS module initialized (waiting for fix...)");

  // Initialize SIM800L on UART1
  Serial.println("🔧 Initializing SIM800L...");
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(3000);  // Give SIM800L time to boot up
  
  // Test SIM800L connectivity
  if (sim800Ready()) {
    Serial.println("✅ SIM800L ready and registered on network");
  } else {
    Serial.println("⚠️  SIM800L not ready - check power and SIM card");
  }

  Serial.println("\n========================================");
  Serial.println("   SMART HELMET SYSTEM READY          ");
  Serial.println("   Monitoring for accidents...        ");
  Serial.println("========================================\n");
}

/* ================= LOOP ================= */

void loop() {

  // ===== CANCEL BUTTON CHECK =====
  // Allow user to cancel false alarm by pressing button
  if (digitalRead(CANCEL_BTN) == LOW) {
    crashDetected = false;
    smsSent = false;
    showingSMSMessage = false;
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("❌ ALERT CANCELLED BY USER");
    delay(1000);  // Debounce delay
  }
  
  // Check if SMS message should be cleared after 2 seconds
  if (showingSMSMessage && (millis() - smsDisplayTime >= 2000)) {
    showingSMSMessage = false;
    updateOLEDDisplay();
  }

  // ===== READ SENSOR DATA =====
  readMPU();

  // Convert raw accelerometer values to g-force (1g = 16384 LSB at ±2g scale)
  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  // Convert raw gyroscope values to degrees per second (dps)
  // At ±250°/s scale: 131 LSB/(°/s)
  // Subtract bias calculated during calibration
  float gx = (GyX - gyroBiasX) / 131.0;
  float gy = (GyY - gyroBiasY) / 131.0;
  float gz = (GyZ - gyroBiasZ) / 131.0;

  // Calculate magnitude of acceleration and rotation vectors
  float accMag = sqrt(ax*ax + ay*ay + az*az);
  float gyroMag = sqrt(gx*gx + gy*gy + gz*gz);

  // Display real-time sensor readings
  Serial.print("ACC(g): "); Serial.print(accMag, 2);
  Serial.print(" | GYRO(dps): "); Serial.print(gyroMag, 1);

  // ===== CRASH DETECTION LOGIC =====
  // A crash is characterized by:
  // 1. High acceleration (sudden jerk/impact)
  // 2. High rotation (bike/helmet tilting)
  // 3. Both conditions sustained for IMPACT_TIME_MS
  
  if (accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    // Potential crash detected - start timing
    if (impactStart == 0) {
      impactStart = millis();
      Serial.print(" [IMPACT DETECTED]");
    }
    
    // Verify impact sustained for minimum time to avoid false triggers
    if (millis() - impactStart > IMPACT_TIME_MS) {
      if (!crashDetected) {
        crashDetected = true;
        digitalWrite(BUZZER_PIN, HIGH);  // Activate buzzer
        Serial.println("\n🚨🚨🚨 CRASH CONFIRMED 🚨🚨🚨");
      }
    }
  } else {
    // Conditions no longer met - reset timer
    impactStart = 0;
  }
  
  Serial.println();  // New line for readability

  // ===== GPS DATA PROCESSING =====
  // Continuously feed GPS data to parser
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  
  // Update GPS connection status
  gpsConnected = gps.location.isValid();

  // ===== EMERGENCY SMS TRANSMISSION =====
  // Only send SMS once when:
  // 1. Crash has been detected
  // 2. GPS has valid location fix
  // 3. SMS hasn't been sent yet
  if (crashDetected && gps.location.isValid() && !smsSent) {
    Serial.println("📍 GPS fix acquired - sending emergency SMS...");
    if (sendSMS(gps.location.lat(), gps.location.lng())) {
      smsSent = true;
      showingSMSMessage = true;
      smsDisplayTime = millis();
      updateOLEDDisplay();
      // Keep buzzer on until user cancels
    } else {
      Serial.println("⚠️  SMS failed - will retry...");
      delay(5000);  // Wait before retry
    }
  }

  // ===== GPS STATUS MONITORING =====
  // Inform user if waiting for GPS fix after crash
  if (crashDetected && !gps.location.isValid() && !smsSent) {
    Serial.print("⏳ Waiting for GPS fix... ");
    Serial.print("Satellites: "); Serial.println(gps.satellites.value());
  }

  // Display GPS info when available
  if (gps.location.isValid()) {
    Serial.print("📍 GPS: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.print(gps.location.lng(), 6);
    Serial.print(" | Sats: ");
    Serial.println(gps.satellites.value());
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

  delay(200);  // Loop delay for sensor sampling rate (~5 Hz)
}
