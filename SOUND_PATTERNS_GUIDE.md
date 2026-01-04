# Passive Buzzer Sound Reference Guide

## Sound Pattern Overview

This document provides a quick reference for all buzzer sound patterns in the enhanced Smart Helmet system.

## 1. Power-On Notification

**When it plays:** System startup when SIM800L connects to network

**Pattern:** Ascending musical notes
```
C5 (523 Hz) - 200ms
[pause 50ms]
E5 (659 Hz) - 200ms  
[pause 50ms]
G5 (784 Hz) - 300ms
[pause 50ms]
```

**Total Duration:** ~800ms

**Code Location:** `playPowerOnTone()`

**Purpose:** Confirms system is ready and network connected

**Sound Description:** Three ascending beeps (do-mi-sol in music)

---

## 2. Warning Tone (Crash Detected)

**When it plays:** Immediately after crash is confirmed

**Pattern:** Continuous tone
```
A5 (880 Hz) - Continuous
```

**Duration:** 10 seconds

**Code Location:** `startWarningTone()`

**Purpose:** Alert rider and nearby people of crash detection

**Sound Description:** Loud continuous alarm tone

**Transitions to:** Beep pattern (when SMS sending starts)

---

## 3. Beep Pattern (SMS Sending)

**When it plays:** During SMS transmission to 3 contacts

**Pattern:** Rapid beeping
```
E5 (659 Hz) - 200ms ON
[silence] - 200ms OFF
E5 (659 Hz) - 200ms ON
[silence] - 200ms OFF
E5 (659 Hz) - 200ms ON
... (repeating)
```

**Duration:** 3 seconds

**Code Location:** `handleBeepPattern()`

**Purpose:** Indicate SMS is being sent

**Sound Description:** Beep-beep-beep-beep pattern

---

## Buzzer State Machine

```
SYSTEM START
     |
     v
[Check SIM800L] --> Not Ready --> (No Sound)
     |
     v
   Ready
     |
     v
[POWER-ON TONE] (C5-E5-G5, ~800ms)
     |
     v
[MONITORING MODE] (No Sound)
     |
     v
[CRASH DETECTED]
     |
     v
[WARNING TONE] (A5 continuous, 10s)
     |
     v
[GPS FIX ACQUIRED]
     |
     v
[BEEP PATTERN] (E5 beep-beep, 3s)
     |
     v
[SMS SENT] --> Silence (or cancelled by button)
```

---

## Musical Note Frequencies

| Note | Frequency | Used In |
|------|-----------|---------|
| C5 | 523 Hz | Power-On (1st note) |
| D5 | 587 Hz | (Reserved) |
| E5 | 659 Hz | Power-On (2nd note), Beep Pattern |
| F5 | 698 Hz | (Reserved) |
| G5 | 784 Hz | Power-On (3rd note) |
| A5 | 880 Hz | Warning Tone |
| B5 | 988 Hz | (Reserved) |
| C6 | 1047 Hz | (Reserved) |

---

## Code Examples

### Playing a Tone
```cpp
// Play 880 Hz for 1 second
tone(BUZZER_PIN, 880, 1000);
delay(1000);

// Or play continuously
tone(BUZZER_PIN, 880);
// ... later ...
noTone(BUZZER_PIN);
```

### Custom Sound Pattern
```cpp
void playCustomSound() {
  tone(BUZZER_PIN, NOTE_C5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_E5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_G5, 200);
  delay(250);
  noTone(BUZZER_PIN);
}
```

---

## Troubleshooting Sounds

### Sound Too Quiet
- Check buzzer polarity
- Add transistor driver circuit
- Increase PWM duty cycle

### No Sound at All
- Verify passive buzzer (not active)
- Check GPIO 25 connection
- Test with simple example:
  ```cpp
  void loop() {
    tone(BUZZER_PIN, 1000, 500);
    delay(1000);
  }
  ```

### Distorted Sound
- Check power supply stability
- Add decoupling capacitor
- Reduce frequency or duration

---

## Customization Ideas

### Change Warning Tone Frequency
```cpp
#define TONE_WARNING NOTE_C6  // Higher pitch
// or
#define TONE_WARNING 440      // Lower pitch (A4)
```

### Extend Warning Duration
```cpp
if (millis() - warningToneStart >= 15000) {  // 15 seconds instead of 10
  stopWarningToneStartBeep();
}
```

### Add Additional Patterns
```cpp
void playAlertPattern() {
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, NOTE_A5, 100);
    delay(150);
    tone(BUZZER_PIN, NOTE_C6, 100);
    delay(150);
  }
  noTone(BUZZER_PIN);
}
```

### Different Beep Pattern
```cpp
// Faster beeps
unsigned long cycleTime = elapsed % 300;  // 300ms cycle instead of 400ms
if (cycleTime < 150) {  // 150ms on
  tone(BUZZER_PIN, TONE_BEEP);
} else {  // 150ms off
  noTone(BUZZER_PIN);
}
```

---

## Passive vs Active Buzzer

### Passive Buzzer (This Project)
- ✅ Requires square wave signal (PWM)
- ✅ Can produce different tones/frequencies
- ✅ More versatile for alerts
- ✅ Musical notes possible
- ❌ Slightly more complex code

### Active Buzzer (Original Project)
- ✅ Simple ON/OFF control
- ✅ Fixed frequency tone
- ✅ Simpler code
- ❌ Cannot change pitch
- ❌ Less informative alerts

---

## Sound Level Considerations

### Typical Passive Buzzer Volume
- **Sound Pressure:** 80-95 dB at 10cm
- **Effective Range:** 2-5 meters
- **Audible through helmet:** Yes (with proper placement)

### Placement Recommendations
1. **Inside helmet:** Near ear, padded to prevent discomfort
2. **Outside helmet:** Near ventilation holes for louder output
3. **Electronics box:** If helmet is nearby when parked

### Legal Considerations
- Check local regulations for audible alarms on helmets
- Some regions may restrict certain sound levels
- Emergency vehicle siren frequencies may be restricted

---

## Testing Checklist

- [ ] Power-on tone plays on startup
- [ ] Warning tone starts immediately on crash
- [ ] Warning tone lasts 10 seconds
- [ ] Beep pattern starts after warning
- [ ] Beep pattern lasts 3 seconds
- [ ] All tones audible and clear
- [ ] No distortion at any frequency
- [ ] Cancel button stops all sounds
- [ ] Sounds work after system reset

---

## Quick Reference Card (Print & Laminate)

```
╔══════════════════════════════════════╗
║   SMART HELMET - SOUND GUIDE         ║
╠══════════════════════════════════════╣
║ POWER-ON:                            ║
║   ♪ Do-Mi-Sol (ascending)            ║
║   → System ready                     ║
║                                      ║
║ CRASH WARNING:                       ║
║   ♪ Continuous alarm (10 sec)        ║
║   → Crash detected                   ║
║                                      ║
║ SMS SENDING:                         ║
║   ♪ Beep-beep-beep (3 sec)           ║
║   → Emergency SMS being sent         ║
║                                      ║
║ SILENCE:                             ║
║   → Normal monitoring                ║
║   → Press button to cancel           ║
╚══════════════════════════════════════╝
```

---

## Additional Resources

- Arduino `tone()` documentation: https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/
- Musical note frequencies: https://pages.mtu.edu/~suits/notefreqs.html
- ESP32 PWM/LEDC: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html

---

**Last Updated:** 2024
**Version:** 2.0
