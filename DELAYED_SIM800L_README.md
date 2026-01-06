# Delayed SIM800L - Crash Detection with Buzzer Priority

## Overview

This is a modified version of the smart helmet crash detection code that solves power issues with the SIM800L module by prioritizing buzzer activation before SMS transmission.

## Problem Statement

**Original Issue:**
- When a crash is detected, the system attempts to activate the buzzer and send SMS simultaneously
- SIM800L TX transmission is a high power event (can draw up to 2A in bursts)
- This power surge causes the buzzer to fail to turn on due to voltage drop

## Solution

**Modified Crash Logic:**
1. **Start buzzer FIRST** - Activate buzzer immediately when crash is detected
2. **Delay 300-500ms** - Wait for buzzer to stabilize and power to settle (implementation uses 400ms)
3. **THEN send SMS** - Initiate SIM800L transmission after buzzer is securely on

## File Details

**Filename:** `delayed-sim800l.ino`

**Key Features:**
- Buzzer-first crash handling sequence
- 400ms delay between buzzer and SMS (middle of 300-500ms range)
- Based on the original `sim800L.ino` test code
- Includes automatic crash simulation for testing
- Clear serial output showing each step of the process

## Hardware Setup

### Required Components:
- ESP32 Development Board
- SIM800L GSM/GPRS Module
- Buzzer (Active or Passive)
- Stable power supply (4.2V for SIM800L, preferably with capacitors for smoothing)

### Pin Configuration:
```
SIM800L:
  - RX  -> GPIO 17 (ESP32 TX2)
  - TX  -> GPIO 16 (ESP32 RX2)
  - VCC -> 4.2V (use buck converter with good capacitors)
  - GND -> GND

Buzzer:
  - Positive -> GPIO 25
  - Negative -> GND
```

## Configuration

### Timing Configuration
You can adjust the timing constants at the top of the file:

```cpp
#define BUZZER_DELAY_MS 400      // Delay between buzzer and SMS (300-500ms range)
#define BUZZER_DURATION_MS 5000  // How long to keep buzzer on after crash
```

### Phone Number Configuration
Before uploading, change the phone number in the code:

```cpp
const char PHONE_NUMBER[] = "+1234567890";  // ⚠️ CHANGE TO YOUR EMERGENCY CONTACT (REQUIRED)
```

**IMPORTANT:** The default phone number is a placeholder. You MUST change it to your actual emergency contact number before deploying the system.

## Testing

The code includes automatic crash simulation:
1. Upload the code to ESP32
2. Open Serial Monitor (115200 baud)
3. The system will automatically simulate a crash after 5 seconds
4. Observe the sequence:
   - Buzzer activates
   - 400ms delay
   - SMS is sent
5. Buzzer stays on for 5 more seconds, then turns off

### Expected Serial Output:
```
========================================
  DELAYED SIM800L - CRASH TEST         
  Buzzer-First Logic Implementation    
========================================

✅ Buzzer pin configured
🔧 Initializing SIM800L...
✅ SIM800L initialized

========================================
   SYSTEM READY - Testing Crash Logic  
========================================

⚠️  SIMULATING CRASH EVENT IN 3 SECONDS...

🚨🚨🚨 CRASH DETECTED 🚨🚨🚨
🔊 Activating buzzer...
⏱️  Delaying 400ms before SMS...
📱 Power-hungry SIM800L TX starting now...
📤 Sending emergency SMS...
📨 SMS SENT
✅ Crash handling complete
Buzzer: ON | SMS: SENT

🔇 Buzzer turned off

========================================
   TEST COMPLETE                        
========================================
```

## Integration with Full System

To integrate this logic into a full smart helmet system (with MPU6050, GPS, etc.):

1. Replace the crash detection logic in your main code with the `handleCrashDetected()` function from this file
2. Ensure the sequence is maintained:
   - Buzzer activation
   - 400ms delay
   - SMS transmission
3. Keep the buzzer on until user cancels or assistance arrives

### Example Integration:
```cpp
// In your main loop, when crash is detected:
if (crashDetected && gps.location.isValid() && !smsSent) {
  Serial.println("📍 GPS fix acquired - handling crash...");
  
  // STEP 1: Start buzzer FIRST
  digitalWrite(BUZZER_PIN, HIGH);
  
  // STEP 2: Delay 400ms
  delay(400);
  
  // STEP 3: THEN send SMS
  if (sendSMS(gps.location.lat(), gps.location.lng())) {
    smsSent = true;
  }
}
```

## Power Considerations

### Why This Sequence Matters:
- **SIM800L TX current:** 
  - Peak: Up to 2A during transmission bursts (lasting ~577ms per burst)
  - Average during call/SMS: ~300-500mA
  - Idle: ~10mA
- **Buzzer current:** Typically 30-50mA
- **Issue:** If both activate simultaneously, voltage drop can prevent buzzer from latching

### Recommended Power Setup:
1. Use a dedicated buck converter for SIM800L (4.2V, 2A+ capacity)
2. Add 1000µF+ capacitor near SIM800L VCC pin
3. Ensure proper grounding between all modules
4. Use separate 5V rail for ESP32 and buzzer if possible

## Troubleshooting

**Buzzer still not turning on?**
- Increase delay to 500ms: `delay(500);`
- Check power supply capacity
- Add larger capacitors to SIM800L power line
- Verify buzzer is working (test separately)

**SMS not sending?**
- Check SIM card is inserted and has credit
- Verify phone number format (include country code)
- Check network registration in serial output
- Ensure SIM800L has adequate power

## Version History

- **v1.0** - Initial implementation with 400ms delay
- Based on original `sim800L.ino` test code
- Created to solve buzzer power issues during crash detection

## Author

Smart Helmet Project
Modified for buzzer-first crash logic implementation

## Related Files

- `sim800L.ino` - Original SIM800L test code
- `new-full-code.ino` - Full system implementation
- `final-code.ino` - Enhanced system with web dashboard
