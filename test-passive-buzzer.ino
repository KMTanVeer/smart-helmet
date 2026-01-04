/*
 * Passive Buzzer Test - Smart Helmet
 * ===================================
 * 
 * This simple test verifies your passive buzzer is working correctly
 * before uploading the full smart helmet code.
 * 
 * HARDWARE REQUIRED:
 * - ESP32 board
 * - Passive buzzer (2-pin or 3-pin without driver)
 * 
 * WIRING:
 * - Buzzer Positive (+) -> GPIO 25
 * - Buzzer Negative (-) -> GND
 * 
 * WHAT TO EXPECT:
 * 1. Power-on melody (C-E-G ascending)
 * 2. Warning tone (continuous 880 Hz)
 * 3. Beep pattern (rapid beeps)
 * 4. Repeat cycle
 * 
 * If you hear these sounds, your buzzer is working!
 */

#define BUZZER_PIN 25

// Musical notes
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_A5  880

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  
  Serial.println("\n====================================");
  Serial.println("  PASSIVE BUZZER TEST - SMART HELMET");
  Serial.println("====================================\n");
  Serial.println("If you hear the sounds, your buzzer works!");
  Serial.println("If not, check:");
  Serial.println("  - Using PASSIVE buzzer (not active)");
  Serial.println("  - Correct wiring (+ to GPIO 25, - to GND)");
  Serial.println("  - Buzzer is not damaged\n");
  
  delay(2000);
}

void loop() {
  // Test 1: Power-On Melody
  Serial.println("🔊 Test 1: Power-On Melody (ascending notes)");
  tone(BUZZER_PIN, NOTE_C5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_E5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_G5, 300);
  delay(350);
  noTone(BUZZER_PIN);
  
  Serial.println("   ✅ Power-on melody played");
  delay(2000);
  
  // Test 2: Warning Tone (10 seconds)
  Serial.println("🚨 Test 2: Warning Tone (continuous 880 Hz for 3 seconds)");
  tone(BUZZER_PIN, NOTE_A5);  // Continuous
  delay(3000);  // 3 seconds for test (real code uses 10)
  noTone(BUZZER_PIN);
  
  Serial.println("   ✅ Warning tone played");
  delay(2000);
  
  // Test 3: Beep Pattern (3 seconds)
  Serial.println("🔔 Test 3: Beep Pattern (beep-beep-beep)");
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) {
    unsigned long elapsed = millis() - startTime;
    unsigned long cycleTime = elapsed % 400;
    
    if (cycleTime < 200) {
      tone(BUZZER_PIN, NOTE_E5);
    } else {
      noTone(BUZZER_PIN);
    }
    delay(10);
  }
  noTone(BUZZER_PIN);
  
  Serial.println("   ✅ Beep pattern played");
  delay(2000);
  
  // Test 4: Different Frequencies
  Serial.println("🎵 Test 4: Frequency Sweep (200Hz to 2000Hz)");
  for (int freq = 200; freq <= 2000; freq += 50) {
    tone(BUZZER_PIN, freq, 50);
    delay(60);
  }
  noTone(BUZZER_PIN);
  
  Serial.println("   ✅ Frequency sweep completed");
  Serial.println("\n✅ ALL TESTS COMPLETED!");
  Serial.println("====================================\n");
  
  delay(5000);  // Wait before repeating
}
