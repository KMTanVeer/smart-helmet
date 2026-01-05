# Passive Buzzer Implementation - first-code-passive-buzzer.ino

## Overview

This is an enhanced version of `full-code.ino` that uses a **passive buzzer** to provide audio feedback during system operation and crash detection. The passive buzzer can produce different tones and melodies, making the alerts more informative.

## New Features

### 1. Power-On Melody
- **When**: Plays when the system starts and SIM800L connects to network
- **Sound**: Ascending musical notes (C5 → E5 → G5)
- **Duration**: ~800ms
- **Purpose**: Confirms system is ready and operational

### 2. 10-Second Warning Tone
- **When**: Immediately after crash is detected
- **Sound**: Continuous 880 Hz (A5) alarm tone
- **Duration**: 10 seconds
- **Purpose**: Alert the rider and nearby people of crash detection

### 3. 3-Second Beep Pattern
- **When**: After the 10-second warning tone completes
- **Sound**: Beep-beep-beep (200ms on, 200ms off)
- **Duration**: 3 seconds
- **Purpose**: Indicate SMS is about to be sent

### 4. SMS Abort Feature
- **Function**: Press the button during the 3-second beep pattern to abort SMS
- **Effect**: Prevents SMS from being sent (useful for false positives)
- **Feedback**: System logs "SMS ABORTED BY USER DURING BEEP PATTERN"

## Sound Sequence Flow

```
SYSTEM START
     |
     v
[SIM800L Ready] → Play Power-On Melody (C5-E5-G5)
     |
     v
[MONITORING MODE] → No Sound (Silent)
     |
     v
[CRASH DETECTED]
     |
     v
[WARNING TONE] → 880 Hz continuous for 10 seconds
     |
     v
[BEEP PATTERN] → Beep-beep-beep for 3 seconds
     |           (Button press here aborts SMS)
     v
[GPS FIX + NO ABORT] → Send SMS
     |
     v
[SMS SENT] → Silence
```

## Key Differences from full-code.ino

| Feature | full-code.ino | first-code-passive-buzzer.ino |
|---------|---------------|-------------------------------|
| Buzzer Type | Active (ON/OFF only) | Passive (Multiple tones) |
| Power-On Sound | None | Musical melody (C5-E5-G5) |
| Crash Alert | Immediate buzzer ON | 10-second warning tone |
| SMS Timing | Immediate after GPS fix | After 10s warning + 3s beep |
| Button Cancel | Anytime | Most effective during beep pattern |
| Sound Patterns | Single tone | Multiple frequencies |

## Hardware Requirements

### Passive Buzzer
- **Type**: Passive buzzer (requires PWM signal)
- **Connection**: 
  - Positive (+) → GPIO 25
  - Negative (-) → GND
- **Note**: Do NOT use an active buzzer (with built-in oscillator)

### Wiring Verification
```
Passive Buzzer:
  - If it has 2 pins: Connect + to GPIO 25, - to GND
  - If it has 3 pins: Usually + (VCC), - (GND), S (Signal to GPIO 25)
  - Test: Upload test-passive-buzzer.ino first to verify it works
```

## Musical Notes Used

| Note | Frequency | Usage |
|------|-----------|-------|
| C5 | 523 Hz | Power-on (1st note) |
| E5 | 659 Hz | Power-on (2nd note), Beep pattern |
| G5 | 784 Hz | Power-on (3rd note) |
| A5 | 880 Hz | Warning tone |

## Configuration

Before uploading:

1. **Change Phone Number**:
   ```cpp
   const char PHONE_NUMBER[] = "+1234567890";  // Your emergency contact
   ```

2. **Adjust Thresholds** (optional):
   ```cpp
   #define ACC_THRESHOLD   2.5    // g-force threshold
   #define GYRO_THRESHOLD  120.0  // rotation threshold (deg/s)
   ```

3. **Adjust Timing** (optional):
   ```cpp
   #define WARNING_TONE_DURATION 10000  // 10 seconds
   #define BEEP_PATTERN_DURATION 3000   // 3 seconds
   ```

## Testing the Code

### Step 1: Test Passive Buzzer Hardware
```bash
# Upload test-passive-buzzer.ino first
# Listen for power-on melody, warning tone, and beep pattern
# If you hear these sounds clearly, proceed to step 2
```

### Step 2: Upload first-code-passive-buzzer.ino
- Connect all sensors (MPU6050, GPS, SIM800L)
- Open Serial Monitor (115200 baud)
- Wait for system initialization
- Listen for power-on melody when SIM800L connects

### Step 3: Test Crash Detection
- Shake/tilt the helmet to simulate crash
- Should hear 10-second warning tone
- Then 3-second beep pattern
- Press button during beep to test abort
- Check Serial Monitor for "SMS ABORTED" message

## Troubleshooting

### No Sound at Power-On
- Check if SIM800L is connected to network
- Power-on melody only plays when SIM800L is ready
- Check serial output for "SIM800L ready and registered"

### Buzzer Too Quiet
- Passive buzzers are quieter than active buzzers
- Ensure correct wiring (+ to GPIO 25, - to GND)
- Try different passive buzzer model
- Consider adding transistor driver circuit

### No Sound at All
- Verify you're using PASSIVE buzzer (not active)
- Test with test-passive-buzzer.ino
- Check GPIO 25 connection
- Verify buzzer polarity

### SMS Not Aborting
- Ensure button is pressed DURING the 3-second beep pattern
- Button press must be held until you see log message
- Check button wiring (GPIO 33 to GND with internal pullup)

## Code Structure

### State Machine
```cpp
enum BuzzerState {
  BUZZER_IDLE,          // No sound (monitoring)
  BUZZER_WARNING_TONE,  // 10-second continuous warning
  BUZZER_BEEP_PATTERN,  // 3-second beep pattern
  BUZZER_SILENT         // After SMS sent or cancelled
};
```

### Main Functions
- `playPowerOnTone()` - Plays C5-E5-G5 melody on startup
- `startWarningTone()` - Starts 10-second 880 Hz tone
- `startBeepPattern()` - Starts 3-second beep pattern
- `handleBeepPattern()` - Maintains beep timing (200ms on/off)
- `stopBuzzer()` - Stops all buzzer sounds

## Timeline Example

```
T=0s    : Crash detected → Start warning tone (880 Hz)
T=10s   : Warning tone ends → Start beep pattern (beep-beep-beep)
T=10-13s: Beep pattern playing (BUTTON CAN ABORT HERE)
T=13s   : If not aborted and GPS fix → Send SMS
T=13s+  : Silence (SMS sent or aborted)
```

## Safety Notes

1. **Test thoroughly** before deploying on actual helmet
2. **Adjust thresholds** based on your riding conditions
3. **Keep button accessible** for quick abort in false alarms
4. **Check local laws** regarding audible alerts on helmets
5. **Verify SMS sending** works before relying on it

## Libraries Required

- Wire (built-in)
- TinyGPSPlus (by Mikal Hart) - Install via Arduino Library Manager
- Adafruit_GFX (for OLED display)
- Adafruit_SSD1306 (for OLED display)

## Version Information

- **Filename**: first-code-passive-buzzer.ino
- **Version**: 2.0 - Passive Buzzer
- **Based On**: full-code.ino
- **Author**: Smart Helmet Project
- **Date**: 2024

## Support

For issues or questions:
1. Check test-passive-buzzer.ino works first
2. Review SOUND_PATTERNS_GUIDE.md for passive buzzer details
3. Verify all hardware connections
4. Check serial output for error messages

## Next Steps

After successful testing:
- Fine-tune ACC_THRESHOLD and GYRO_THRESHOLD for your needs
- Adjust WARNING_TONE_DURATION if needed (default 10s)
- Adjust BEEP_PATTERN_DURATION if needed (default 3s)
- Consider adding battery level monitoring (code includes TODO)
- Test in real-world riding conditions (in safe environment)
