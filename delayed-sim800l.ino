/* 
 * =====================================================
 * DELAYED SIM800L - CRASH DETECTION WITH BUZZER PRIORITY
 * =====================================================
 * 
 * This version prioritizes buzzer activation before SMS transmission
 * to solve power issues with the SIM800L module.
 * 
 * MODIFICATION:
 * When crash is detected:
 * 1. Start buzzer FIRST
 * 2. Delay 400ms (300-500ms range)
 * 3. THEN send SMS
 * 
 * Reason: SIM800L TX is a high power event that can cause the buzzer 
 * to fail to turn on due to power supply issues.
 */

#include <HardwareSerial.h>

/* ================= HARDWARE SERIAL ================= */
HardwareSerial sim800(2);   // UART2 for SIM800L

/* ================= PIN DEFINITIONS ================= */
#define SIM_RX 16   // ESP32 RX2 <- SIM800 TX
#define SIM_TX 17   // ESP32 TX2 -> SIM800 RX
#define BUZZER_PIN 25   // Buzzer pin

/* ================= CONFIGURATION ================= */
const char PHONE_NUMBER[] = "+8801747213525";  // ⚠️ CHANGE TO YOUR EMERGENCY CONTACT

/* ================= UTILITIES ================= */

// Send AT command and wait for response
void sendCmd(const char* cmd, unsigned long waitMs = 1500) {
  sim800.println(cmd);
  delay(waitMs);
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
  Serial.println();
}

// Initialize SIM800L module
void initSIM800L() {
  Serial.println("🔧 Initializing SIM800L...");
  
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(6000); // Allow SIM800L to boot fully
  
  // Basic initialization
  sendCmd("AT");
  sendCmd("ATE0");
  sendCmd("AT+CPIN?");
  sendCmd("AT+CSQ");
  sendCmd("AT+CREG?");
  sendCmd("AT+CMGF=1");          // SMS text mode
  sendCmd("AT+CSCS=\"GSM\"");    // GSM charset
  
  Serial.println("✅ SIM800L initialized");
}

// Send emergency SMS
void sendEmergencySMS() {
  Serial.println("📤 Sending emergency SMS...");
  
  sim800.print("AT+CMGS=\"");
  sim800.print(PHONE_NUMBER);
  sim800.print("\"\r");
  delay(1000);
  
  sim800.print("🚨 CRASH DETECTED 🚨\nEmergency alert from Smart Helmet.\nImmediate assistance required!");
  delay(500);
  
  sim800.write(26);  // CTRL+Z to send
  delay(5000);
  
  Serial.println("📨 SMS SENT");
}

/* ================= CRASH DETECTION WITH BUZZER PRIORITY ================= */

void handleCrashDetected() {
  Serial.println("\n🚨🚨🚨 CRASH DETECTED 🚨🚨🚨");
  
  // STEP 1: Start buzzer FIRST
  Serial.println("🔊 Activating buzzer...");
  digitalWrite(BUZZER_PIN, HIGH);
  
  // STEP 2: Delay 400ms (middle of 300-500ms range)
  Serial.println("⏱️  Delaying 400ms before SMS...");
  delay(400);
  
  // STEP 3: THEN send SMS
  Serial.println("📱 Power-hungry SIM800L TX starting now...");
  sendEmergencySMS();
  
  Serial.println("✅ Crash handling complete");
  Serial.println("Buzzer: ON | SMS: SENT");
}

/* ================= SETUP ================= */

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n========================================");
  Serial.println("  DELAYED SIM800L - CRASH TEST         ");
  Serial.println("  Buzzer-First Logic Implementation    ");
  Serial.println("========================================\n");
  
  // Configure buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);  // Ensure buzzer is off initially
  Serial.println("✅ Buzzer pin configured");
  
  // Initialize SIM800L
  initSIM800L();
  
  Serial.println("\n========================================");
  Serial.println("   SYSTEM READY - Testing Crash Logic  ");
  Serial.println("========================================\n");
  
  // Simulate a crash event for testing
  delay(2000);
  Serial.println("⚠️  SIMULATING CRASH EVENT IN 3 SECONDS...\n");
  delay(3000);
  
  // Handle the simulated crash
  handleCrashDetected();
  
  // Keep buzzer on for 5 more seconds, then turn off
  delay(5000);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("\n🔇 Buzzer turned off");
  
  Serial.println("\n========================================");
  Serial.println("   TEST COMPLETE                        ");
  Serial.println("========================================");
}

/* ================= LOOP ================= */

void loop() {
  // Test is complete - nothing to do in loop
  // In real implementation, this would contain sensor monitoring
}
