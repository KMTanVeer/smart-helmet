# Smart Helmet Enhanced Version - Documentation

## Overview

This is the enhanced version of the Smart Helmet system with the following new features:

1. **Passive Buzzer with Sound Patterns** - Instead of active buzzer
2. **Multiple Emergency Contacts** - Send SMS to 3 contacts
3. **Working Battery Monitoring** - Real-time battery voltage reading
4. **Crash Data Logging** - Store crash data in SPIFFS file system

## New File: `smart-helmet-enhanced.ino`

This file contains the complete enhanced implementation. The original `full-code.ino` remains untouched.

## Key Enhancements

### 1. Passive Buzzer Implementation

#### Hardware Change
- **Old**: Active buzzer (simple ON/OFF)
- **New**: Passive buzzer (can generate different tones/frequencies)

#### Sound Patterns

**Power-On Notification**
- Plays when SIM800L successfully connects to network
- Sequence: C5 → E5 → G5 (ascending musical notes)
- Duration: ~800ms total
- Function: `playPowerOnTone()`

**Warning Tone (After Crash Detection)**
- Continuous tone at 880 Hz (A5 note)
- Duration: 10 seconds
- Starts immediately when crash is confirmed
- Function: `startWarningTone()`

**Beep Pattern (During SMS Sending)**
- Rapid beep-beep-beep pattern
- Frequency: 659 Hz (E5 note)
- Pattern: 200ms ON, 200ms OFF
- Duration: 3 seconds
- Function: `handleBeepPattern()`

#### Code Structure
```cpp
// Tone definitions
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_A5  880
// ... etc

// Usage
tone(BUZZER_PIN, NOTE_C5, 200);  // Play C5 for 200ms
noTone(BUZZER_PIN);              // Stop tone
```

### 2. Three Emergency Contacts

#### Configuration
```cpp
const char PHONE_NUMBER_1[] = "+1234567890";   // Contact 1
const char PHONE_NUMBER_2[] = "+0987654321";   // Contact 2
const char PHONE_NUMBER_3[] = "+1122334455";   // Contact 3
```

#### Implementation
- Function: `sendSMSToAllContacts(lat, lon)`
- Sends same emergency message to all 3 numbers sequentially
- 2-second delay between messages to avoid network congestion
- Returns true if at least 1 SMS sent successfully
- Logs success count to serial monitor

#### SMS Format (Same for All Contacts)
```
🚨 ACCIDENT DETECTED!
Emergency alert from Smart Helmet.

Location Details:
Latitude: XX.XXXXXX
Longitude: XX.XXXXXX

View on Google Maps:
https://maps.google.com/?q=XX.XXXXXX,XX.XXXXXX
```

### 3. Battery Monitoring

#### Hardware Setup

**Voltage Divider Circuit:**
```
Battery+ ----[100K resistor]---- GPIO 34 (ADC) ----[100K resistor]---- GND
```

This creates a 2:1 voltage divider:
- Battery voltage: 3.3V - 4.2V
- After divider: 1.65V - 2.1V (safe for ESP32 ADC)

#### Pin Configuration
```cpp
#define BATTERY_PIN 34  // ADC1_CH6
```

#### Voltage Mapping
- **4.2V** → 100% (Fully charged Li-ion)
- **3.7V** → ~45% (Nominal voltage)
- **3.3V** → 0% (Empty/cutoff)

#### Reading Function
```cpp
int readBatteryPercent() {
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = (adcValue / 4095.0) * 3.3;
  float batteryVoltage = voltage * VOLTAGE_DIVIDER_RATIO;
  int percent = map voltage to 0-100%
  return constrain(percent, 0, 100);
}
```

#### Update Frequency
- Battery level read every 10 seconds
- Prevents unnecessary ADC reads and extends ADC lifespan
- Updates OLED display when value changes

### 4. Crash Data Logging

#### Storage Method
- Uses ESP32 SPIFFS (SPI Flash File System)
- File: `/crashes.csv`
- Format: CSV (Comma-Separated Values)

#### Data Structure
```cpp
struct CrashData {
  unsigned long timestamp;  // millis() since boot
  float latitude;
  float longitude;
  float accMagnitude;
  float gyroMagnitude;
  int batteryLevel;
};
```

#### CSV Format
```csv
Timestamp,Latitude,Longitude,AccMag,GyroMag,Battery%
12345,23.810332,90.412518,3.45,156.7,85
```

#### Functions

**Initialize Logging:**
```cpp
void initCrashLogging()
```
- Mounts SPIFFS
- Creates `/crashes.csv` if doesn't exist
- Writes CSV header

**Log Crash Data:**
```cpp
void logCrashData(lat, lon, accMag, gyroMag, battery)
```
- Appends new crash record to CSV file
- Called when crash detected and GPS has fix
- Increments crash counter

**Display Logs:**
```cpp
void displayCrashLogs()
```
- Reads and prints all crash logs to Serial Monitor
- Useful for debugging and data retrieval

#### Retrieving Crash Data

**Method 1: Serial Monitor**
- Call `displayCrashLogs()` in code or via serial command
- Copy data from serial output

**Method 2: SPIFFS File System**
- Use ESP32 filesystem upload tool to download files
- Arduino IDE: Tools → ESP32 Sketch Data Upload (reverse)
- Or use serial file transfer protocol

**Method 3: Web Server (Future Enhancement)**
- Add HTTP server to serve crash log file
- Access via browser: `http://esp32-ip/crashes.csv`

### 5. Graph Generation for Crash Data

#### Current Implementation
- Data stored in CSV format (easy to import)
- Can be imported into:
  - Excel/Google Sheets for graphs
  - Python (matplotlib, pandas)
  - MATLAB
  - Any data visualization tool

#### Potential Future Enhancements

**Option A: Real-time Web Dashboard**
```cpp
// Add ESP32 web server
#include <WebServer.h>
WebServer server(80);

// Serve HTML page with Chart.js
server.on("/", handleRoot);
server.on("/data", handleCrashData);
```
- Display graphs in web browser
- Use Chart.js or similar library
- Update in real-time

**Option B: SD Card Storage + Processing**
- Store data on SD card instead of SPIFFS
- More storage capacity
- Generate graphs on PC after retrieving SD card

**Option C: Data Export via Bluetooth/WiFi**
- Send crash data to smartphone app
- App generates graphs on phone
- Easier data access

**Option D: OLED Mini-Graphs**
- Display simple bar graph on OLED
- Show last 5-10 crashes
- Limited by 128x64 resolution

#### Python Script for Graph Generation (Example)

```python
import pandas as pd
import matplotlib.pyplot as plt

# Read CSV file
df = pd.read_csv('crashes.csv')

# Plot acceleration magnitude over time
plt.figure(figsize=(10, 6))
plt.plot(df['Timestamp'], df['AccMag'], marker='o')
plt.xlabel('Timestamp (ms)')
plt.ylabel('Acceleration (g)')
plt.title('Crash Detection - Acceleration Magnitude')
plt.grid(True)
plt.savefig('crash_acceleration.png')

# Plot GPS locations on map
plt.figure(figsize=(10, 6))
plt.scatter(df['Longitude'], df['Latitude'], c=range(len(df)), cmap='viridis', s=100)
plt.xlabel('Longitude')
plt.ylabel('Latitude')
plt.title('Crash Locations')
plt.colorbar(label='Crash Number')
plt.savefig('crash_locations.png')
```

## Hardware Requirements

### Changed Components
- ✅ **Passive Buzzer** (instead of Active Buzzer)

### Added Components
- ✅ **2x 100K Resistors** (for battery voltage divider)
- ✅ **Wires** for battery connection

### Same Components (From Original)
- ESP32 Development Board
- MPU6050
- Ublox Neo-8M GPS
- SIM800L GSM Module
- Push Button
- OLED Display SSD1306
- Power supply (7.4V Li-ion battery recommended)

## Wiring Diagram Changes

### New Connection: Battery Monitoring
```
Battery Positive (4.2V max)
         |
         R1 (100K)
         |
         +-------- GPIO 34 (ADC)
         |
         R2 (100K)
         |
        GND
```

### Buzzer Connection (No Physical Change)
- Same pin (GPIO 25)
- But **must use PASSIVE buzzer** (3-pin or 2-pin without driver circuit)

### All Other Connections
- Same as original `full-code.ino`
- See main README.md for full wiring details

## Software Requirements

### Arduino Libraries (Same as Original)
- Wire (built-in)
- TinyGPSPlus
- Adafruit_GFX
- Adafruit_SSD1306

### ESP32 Core Libraries (Built-in)
- SPIFFS (file system)

### Installation
All libraries can be installed via Arduino IDE Library Manager:
1. Sketch → Include Library → Manage Libraries
2. Search for each library by name
3. Click Install

## Configuration Steps

### 1. Change Phone Numbers
```cpp
const char PHONE_NUMBER_1[] = "+1234567890";  // Your number
const char PHONE_NUMBER_2[] = "+0987654321";  // Family member
const char PHONE_NUMBER_3[] = "+1122334455";  // Friend
```

### 2. Calibrate Battery Voltage (If Needed)
If using different voltage divider ratio:
```cpp
#define VOLTAGE_DIVIDER_RATIO 2.0  // Adjust based on your resistors
```

Calculation: `RATIO = (R1 + R2) / R2`
- Example: 100K + 100K = 200K, divided by 100K = 2.0

### 3. Adjust Crash Thresholds (Optional)
```cpp
#define ACC_THRESHOLD   2.5    // Increase for less sensitivity
#define GYRO_THRESHOLD  120.0  // Increase for less sensitivity
```

### 4. Upload Code
1. Connect ESP32 via USB
2. Select Board: "ESP32 Dev Module"
3. Select Port: Your ESP32's COM port
4. Click Upload

## Testing Procedures

### 1. Power-On Tone Test
- Upload code
- Reset ESP32
- Wait for SIM800L to register on network (~3-5 seconds)
- **Expected**: Ascending tone (C5-E5-G5) plays

### 2. Crash Detection Test
- Shake helmet rapidly or hit soft surface
- **Expected**: 
  - Serial: "🚨🚨🚨 CRASH CONFIRMED 🚨🚨🚨"
  - Buzzer: Continuous warning tone (880 Hz)
  - Duration: 10 seconds

### 3. SMS Sending Test
- Trigger crash (as above)
- Wait for GPS fix
- **Expected**:
  - Warning tone stops
  - Beep pattern starts (beep-beep-beep)
  - Serial: "📤 Sending SMS to: [number]"
  - Duration: 3 seconds
  - OLED: "SMS SENT SUCCESS!"
  - All 3 contacts receive SMS

### 4. Battery Monitoring Test
- Monitor Serial output
- **Expected**: Every 10 seconds: "🔋 Battery: XX%"
- OLED shows battery icon with percentage
- Test with different battery levels if possible

### 5. Crash Logging Test
- Trigger crash
- Wait for SMS to send
- Open Serial Monitor
- Call `displayCrashLogs()` (add in loop temporarily)
- **Expected**: CSV data printed to serial

### 6. Cancel Button Test
- Trigger crash
- Press cancel button (GPIO 33 to GND)
- **Expected**:
  - Buzzer stops immediately
  - Serial: "❌ ALERT CANCELLED BY USER"
  - System resets to monitoring mode

## Troubleshooting

### Passive Buzzer Not Working

**Problem**: No sound or distorted sound
- **Check**: Using passive buzzer, not active
- **Check**: Polarity correct (+ to GPIO 25, - to GND)
- **Check**: Buzzer rated for 3.3V or 5V operation
- **Solution**: Try different buzzer or add transistor driver

**Problem**: Tone too quiet
- **Solution**: Add small NPN transistor (e.g., 2N2222) driver circuit:
  ```
  GPIO 25 ----[1K]---- Base (NPN)
  GND ------------------- Emitter
  Buzzer+ --------------- Collector
  Buzzer- --------------- GND
  ```

### Battery Reading Issues

**Problem**: Battery shows 0% or 100% always
- **Check**: Voltage divider connected correctly
- **Check**: GPIO 34 not connected to anything else
- **Test**: Measure voltage at GPIO 34 with multimeter (should be 1.65V - 2.1V)
- **Solution**: Adjust `VOLTAGE_DIVIDER_RATIO` in code

**Problem**: Battery percentage jumps around
- **Solution**: Add 10µF capacitor between GPIO 34 and GND (smoothing)
- **Solution**: Increase reading interval (already 10 seconds)
- **Solution**: Add software averaging:
  ```cpp
  int readings[10];
  int avg = (sum of readings) / 10;
  ```

### SPIFFS / Crash Logging Issues

**Problem**: "SPIFFS mount failed"
- **Solution**: In Arduino IDE:
  - Tools → Partition Scheme → "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
  - Re-upload code

**Problem**: "Failed to open crash log file"
- **Check**: SPIFFS initialized in setup()
- **Test**: Add debug prints in `initCrashLogging()`
- **Solution**: Format SPIFFS: `SPIFFS.format()` (erases all data!)

**Problem**: Too many crashes, file full
- **Solution**: Implement log rotation:
  ```cpp
  if (crashLogCount > 50) {
    SPIFFS.remove("/crashes.csv");
    initCrashLogging();
  }
  ```

### SMS Not Sent to All Contacts

**Problem**: Only 1 or 2 contacts receive SMS
- **Check**: All 3 phone numbers correct (with country code)
- **Check**: Signal strength sufficient (>10)
- **Check**: SIM card has SMS credit
- **Solution**: Increase delay between SMS sends (currently 2 seconds)
- **Solution**: Check serial output for specific failures

## Comparison: Original vs Enhanced

| Feature | Original (full-code.ino) | Enhanced (smart-helmet-enhanced.ino) |
|---------|-------------------------|-------------------------------------|
| Buzzer Type | Active (simple ON/OFF) | Passive (tones/melodies) |
| Power-On Sound | ❌ None | ✅ Musical notification |
| Crash Alert Sound | Simple beep | 10s warning + 3s beep pattern |
| Emergency Contacts | 1 contact | 3 contacts |
| Battery Monitoring | TODO comment | ✅ Working (ADC reading) |
| Crash Data Logging | ❌ None | ✅ CSV file in SPIFFS |
| Data Export | ❌ None | ✅ CSV format (graph-ready) |
| OLED Display | ✅ Full featured | ✅ Same (untouched) |
| GPS | ✅ Working | ✅ Same |
| MPU6050 | ✅ Working | ✅ Same |
| Code Size | ~650 lines | ~850 lines (+30%) |

## Performance Considerations

### Memory Usage
- **SPIFFS**: Uses flash memory, not RAM
- **Crash logs**: ~50 bytes per crash record
- **Max crashes**: Limited by SPIFFS size (~1.5MB = ~30,000 crashes)

### Battery Life Impact
- **Passive buzzer**: Similar power consumption to active
- **ADC reading**: Negligible impact (microamps)
- **SPIFFS writes**: Only on crash (minimal)
- **Overall**: No significant battery life reduction

### SMS Sending Time
- **Original**: ~10 seconds (1 contact)
- **Enhanced**: ~20-25 seconds (3 contacts)
- **Network dependent**: May vary

## Future Enhancement Ideas

### 1. Real-time Data Streaming
- Send crash data to cloud (MQTT, HTTP)
- Firebase/AWS for storage
- Real-time dashboard

### 2. Voice Call
- Add voice call to primary contact
- Use SIM800L ATD command
- Parallel with SMS

### 3. Accelerometer Data Buffering
- Record 1-2 seconds before/after crash
- Store detailed sensor data
- Better crash analysis

### 4. Low Power Mode
- Deep sleep when not moving
- Wake on interrupt (MPU6050)
- Extend battery life

### 5. Over-the-Air Updates
- WiFi OTA updates
- Update firmware remotely
- No need to remove helmet electronics

### 6. Mobile App Integration
- Bluetooth connection to smartphone
- Configure settings via app
- View crash logs on phone
- Generate graphs in app

## Files in This Project

- `full-code.ino` - Original working code (UNTOUCHED)
- `smart-helmet-enhanced.ino` - **NEW enhanced version** ⭐
- `ENHANCED_VERSION_README.md` - This documentation
- `README.md` - Original project documentation
- `IMPLEMENTATION_SUMMARY.md` - OLED implementation details
- Other test files (.ino) - Individual component tests

## Summary

The enhanced version successfully implements all requested features:

✅ **Passive buzzer** with distinct sound patterns
✅ **Power-on notification** when system connects
✅ **10-second warning tone** after crash detection  
✅ **3-second beep pattern** during SMS sending
✅ **3 emergency contacts** for SMS alerts
✅ **Working battery monitoring** with ADC reading
✅ **Crash data logging** in CSV format
✅ **Graph-ready data** export
✅ **All original features** preserved (OLED, GPS, etc.)

The original `full-code.ino` remains completely untouched as requested.

## Questions Addressed

**Q: "Do you know how to do that?" (crash data with graphs)**
**A:** Yes! Implemented:
- Crash data stored in CSV format on SPIFFS
- Data includes: timestamp, GPS coordinates, acceleration, gyroscope, battery level
- CSV can be imported into Excel, Python, MATLAB for graphs
- Provided Python example code for graph generation
- Future enhancement options documented (web dashboard, mobile app, etc.)

**Q: "If you need anything to ask..."**
**A:** Implementation complete with reasonable assumptions:
- Used SPIFFS for storage (built-in, no extra hardware)
- CSV format for maximum compatibility
- Voltage divider ratio 2:1 (common setup)
- Can be adjusted based on your specific needs

## Support

For questions or issues:
1. Check troubleshooting section
2. Review serial monitor output
3. Verify hardware connections
4. Test with original `full-code.ino` first to isolate issues

**Stay Safe! 🏍️⛑️**
