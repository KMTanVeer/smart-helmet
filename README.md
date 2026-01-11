# Smart Helmet - Accident Detection & Emergency Alert System

An intelligent motorcycle/bike helmet safety system that automatically detects accidents using motion sensors and sends emergency SMS alerts with GPS location to designated contacts.

## 💝 NEW: OLED Love Quotes Display!

**File: `oled-love-quotes-riya.ino`** - 💕 **Romantic OLED Display** with:
- ❤️ **Animated Hearts** - Pulsing heart icons in all corners
- 💌 **9 Romantic Quotes** - Beautiful messages that rotate
- 🎬 **Smooth Animations** - Professional display effects
- 💝 **Personal Dedication** - "I Love You Riya - from your Tanveer"
- ⚡ **Simple Setup** - Just ESP32 + 0.94" OLED display
- 🎁 **Perfect Gift** - Express your love in a unique way!

👉 See **[OLED_LOVE_QUOTES_README.md](OLED_LOVE_QUOTES_README.md)** for complete guide!  
👉 See **[QUICK_START_LOVE_QUOTES.md](QUICK_START_LOVE_QUOTES.md)** for 5-minute setup!

## 🎯 Not Sure Which Version to Use?

👉 **[VERSION_SELECTION_GUIDE.md](VERSION_SELECTION_GUIDE.md)** - Complete decision guide to help you choose!

## 🧪 NEW: MPU Testing Tool!

**File: `mpu-test-web-dashboard.ino`** - 🧪 **Standalone Testing Tool** for:
- 🔬 **Test MPU6050 crash detection logic** without full system
- 📊 **Real-time web dashboard** with live graphs
- 🔴 **Crashes marked in RED** with "Crash detected" notes
- 💾 **Download data as CSV** to phone/computer
- ⚙️ **Adjustable thresholds** to fine-tune crash detection
- 📱 **Mobile-friendly** interface
- **NO GPS, NO SMS, NO BUZZER** - Pure testing mode!

👉 See **[MPU_TEST_README.md](MPU_TEST_README.md)** for complete testing guide!  
👉 See **[QUICK_START_MPU_TEST.md](QUICK_START_MPU_TEST.md)** for 5-minute setup!

## 🌟 Latest Version - Web Dashboard!

**New File: `smart-helmet-web-dashboard.ino`** - 🔥 **Most Advanced Version** with:
- 🌐 **Real-Time Web Dashboard** - Monitor from mobile/laptop browser
- 📊 **Live Graphs** - Real-time acceleration & gyroscope visualization
- 🔴 **Crash Highlighting** - Crash events shown in RED on graphs
- 📝 **Crash Notes** - Add notes/comments to crash events
- 💾 **CSV Download** - Export complete crash history
- ⚙️ **Configurable Thresholds** - Adjust sensitivity via web interface
- 📱 **Mobile-Friendly** - Responsive design for any device
- ⚡ **WebSocket Streaming** - Instant real-time updates (5 Hz)

👉 See **[WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md)** for complete guide!
👉 See **[LIBRARY_INSTALLATION.md](LIBRARY_INSTALLATION.md)** for library setup!

## 🆕 Enhanced Version Also Available!

**File: `smart-helmet-enhanced.ino`** - Enhanced version with:
- ✨ **Passive Buzzer** with distinct sound patterns (power-on, warning, beep)
- 📱 **3 Emergency Contacts** instead of 1
- 🔋 **Working Battery Monitoring** with real-time percentage display
- 💾 **Crash Data Logging** to SPIFFS with CSV export
- 📊 **Graph Generation** support with Python visualization tools

👉 See **[ENHANCED_VERSION_README.md](ENHANCED_VERSION_README.md)** for full details!

## 🎯 Features

### Original Version (full-code.ino)
- **Automatic Accident Detection**: Uses MPU6050 6-axis gyroscope and accelerometer to detect crashes
- **GPS Location Tracking**: Ublox Neo-8M GPS module provides accurate location coordinates
- **Emergency SMS Alerts**: SIM800L GSM module sends SMS with Google Maps link to emergency contacts
- **Audio Alert**: Buzzer activates when accident is detected
- **False Alarm Prevention**: Manual button to cancel false positives
- **Professional SIM800L Handling**: Robust error checking and network status monitoring
- **Modular Code Design**: Well-organized, documented, and easy to customize
- **OLED Display**: Real-time status display with battery, GPS, and signal strength

## 🔧 Hardware Requirements

### Components

1. **ESP32 Development Board** - Main microcontroller
2. **MPU6050** - 6-axis Gyroscope & Accelerometer
3. **Ublox Neo-8M GPS Module** - Location tracking
4. **SIM800L GSM/GPRS Module** - SMS communication
5. **Active Buzzer** - Audio alert
6. **Push Button** - Cancel false alarms
7. **Power Supply** - Battery pack (7.4V Li-ion recommended)
8. **Buck Converters** - For voltage regulation

### Wiring Diagram

```
ESP32 PIN CONNECTIONS:
======================

MPU6050:
├── VCC  -> 3.3V
├── GND  -> GND
├── SDA  -> GPIO 21
└── SCL  -> GPIO 22

GPS (Ublox Neo-8M):
├── VCC  -> 5V
├── GND  -> GND
├── TX   -> GPIO 16 (ESP32 RX2)
└── RX   -> GPIO 17 (ESP32 TX2)

SIM800L:
├── VCC  -> 4.2V (from buck converter)
├── GND  -> GND (common ground)
├── TX   -> GPIO 26 (ESP32 RX1)
└── RX   -> GPIO 27 (ESP32 TX1)

Buzzer:
├── Positive -> GPIO 25
└── Negative -> GND

Cancel Button:
├── One side -> GPIO 33
└── Other side -> GND (uses internal pullup)
```

## ⚠️ Important Power Supply Notes

### SIM800L Power Requirements
- **Voltage**: 3.4V - 4.4V (4.0V recommended)
- **Current**: Up to 2A peak during transmission
- **Critical**: Use a dedicated power supply or high-capacity battery with buck converter
- **Common Mistakes**:
  - ❌ Powering from ESP32 3.3V pin (insufficient current)
  - ❌ Using USB power without capacitors (voltage drops cause resets)
  - ✅ Use 7.4V battery → Buck converter → 4.0V for SIM800L
  - ✅ Add 100µF + 1000µF capacitors near SIM800L VCC

### Recommended Power Setup
```
7.4V Li-ion Battery Pack
    ├── Buck Converter 1 -> 5V  -> ESP32 VIN, GPS VCC
    └── Buck Converter 2 -> 4.0V -> SIM800L VCC
    
Common GND for all components!
```

## 📚 Software Requirements

### Arduino IDE Setup

1. **Install ESP32 Board Support**:
   - Open Arduino IDE
   - Go to File → Preferences
   - Add to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools → Board → Boards Manager
   - Search "ESP32" and install "esp32 by Espressif Systems"

2. **Install Required Libraries**:
   - **TinyGPSPlus** by Mikal Hart
     - Sketch → Include Library → Manage Libraries
     - Search "TinyGPSPlus" and install
   - **Wire** library (built-in, no installation needed)

3. **Board Configuration**:
   - Board: "ESP32 Dev Module"
   - Upload Speed: 115200
   - Flash Frequency: 80MHz
   - Port: Select your ESP32's COM port

## 🚀 Installation & Setup

### 1. Hardware Assembly

1. Connect all components according to wiring diagram
2. Ensure common ground for all modules
3. Double-check SIM800L power supply (most common issue)
4. Install SIM card in SIM800L (with PIN disabled)
5. Mount components securely in/on helmet

### 2. Software Configuration

1. Open `full-code.ino` in Arduino IDE

2. **Configure Emergency Contact**:
   ```cpp
   const char PHONE_NUMBER[] = "+1234567890";  // Change this!
   ```

3. **Adjust Detection Thresholds** (optional):
   ```cpp
   #define ACC_THRESHOLD   2.5    // g-force sensitivity
   #define GYRO_THRESHOLD  120.0  // rotation sensitivity
   #define IMPACT_TIME_MS  30     // sustained impact time
   ```

4. Upload code to ESP32

### 3. Testing

1. **Initial Test**:
   - Open Serial Monitor (115200 baud)
   - System should display initialization messages
   - Check for "SIM800L ready and registered on network"
   - Wait for "GPS fix acquired"

2. **Gyro Calibration**:
   - Keep helmet completely still during startup
   - Calibration takes ~2 seconds
   - Check calibration values in serial output

3. **Accident Simulation**:
   - Safely simulate impact by hitting padded surface
   - Or manually tilt/shake helmet rapidly
   - Buzzer should activate
   - SMS should be sent when GPS has fix

4. **Cancel Button Test**:
   - Press cancel button during alert
   - Buzzer should stop
   - System should reset to monitoring mode

## 📱 SMS Message Format

When an accident is detected, the following SMS is sent:

```
🚨 ACCIDENT DETECTED!
Emergency alert from Smart Helmet.

Location Details:
Latitude: 23.810332
Longitude: 90.412518

View on Google Maps:
https://maps.google.com/?q=23.810332,90.412518
```

The recipient can click the Google Maps link to view exact location.

## 🔍 Troubleshooting

### SIM800L Issues

**Problem**: "SIM800L not responding to AT"
- Check power supply (4.0V, sufficient current)
- Verify TX/RX connections (not swapped)
- Check common ground
- Add large capacitors near SIM800L

**Problem**: "SIM800L not registered on network"
- Ensure SIM card is inserted correctly
- Disable SIM PIN code
- Check signal strength (move to area with better coverage)
- Verify SIM has active plan with SMS capability

### GPS Issues

**Problem**: "Waiting for GPS fix" for extended time
- GPS needs clear view of sky
- Initial fix can take 1-5 minutes
- Move away from buildings/trees
- Check GPS antenna connection

### MPU6050 Issues

**Problem**: Erratic readings or false detections
- Recalibrate gyroscope (keep very still during startup)
- Check I2C connections (SDA/SCL)
- Adjust thresholds for your use case
- Ensure sensor is firmly mounted

### ESP32 Issues

**Problem**: Code won't upload
- Hold BOOT button while uploading starts
- Check correct COM port selected
- Try different upload speed (921600 or 115200)
- Check USB cable (data capable, not charge-only)

## 🎛️ Customization

### Adjusting Sensitivity

For different riding conditions, adjust these values in code:

```cpp
// More sensitive (detects lighter impacts)
#define ACC_THRESHOLD   2.0
#define GYRO_THRESHOLD  100.0

// Less sensitive (only severe crashes)
#define ACC_THRESHOLD   3.0
#define GYRO_THRESHOLD  150.0
```

### Multiple Emergency Contacts

To send SMS to multiple numbers, modify `sendSMS()` function:

```cpp
const char PHONE_NUMBER1[] = "+1234567890";
const char PHONE_NUMBER2[] = "+0987654321";

// In sendSMS function, call twice:
sendSMS_to_number(lat, lon, PHONE_NUMBER1);
sendSMS_to_number(lat, lon, PHONE_NUMBER2);
```

### Adding Voice Call

For critical emergencies, you can add voice call capability:

```cpp
void makeEmergencyCall() {
  sim800.println("ATD+1234567890;");  // Dial number
  delay(30000);  // Call duration
  sim800.println("ATH");  // Hang up
}
```

## 📊 Serial Monitor Output

During normal operation, you'll see:

```
========================================
   SMART HELMET SYSTEM INITIALIZING   
========================================

🔧 Initializing MPU6050...
✅ MPU6050 initialized
🟡 Calibrating gyro (keep helmet still)...
✅ Gyro calibrated
Bias X: -12.34 Y: 5.67 Z: -8.90
🔧 Initializing GPS...
✅ GPS module initialized (waiting for fix...)
🔧 Initializing SIM800L...
✅ SIM800L ready and registered on network

========================================
   SMART HELMET SYSTEM READY          
   Monitoring for accidents...        
========================================

ACC(g): 1.02 | GYRO(dps): 3.4
📍 GPS: 23.810332, 90.412518 | Sats: 8
ACC(g): 0.98 | GYRO(dps): 2.1
...
```

When crash detected:

```
ACC(g): 3.45 | GYRO(dps): 156.7 [IMPACT DETECTED]
🚨🚨🚨 CRASH CONFIRMED 🚨🚨🚨
📍 GPS fix acquired - sending emergency SMS...
📤 Preparing to send SMS...
📨 SMS SUCCESSFULLY SENT
```

## 🛡️ Safety Considerations

1. **Not a Replacement**: This system supplements, not replaces, proper safety gear
2. **Regular Testing**: Test system monthly to ensure functionality
3. **Battery Maintenance**: Keep battery charged, check connections regularly
4. **Coverage Areas**: System requires cellular network and GPS coverage
5. **Response Time**: SMS delivery not guaranteed - depends on network
6. **False Positives**: Adjust thresholds to balance sensitivity vs false alarms

## 📄 Project Files

### Main Implementation Files
- **`smart-helmet-web-dashboard.ino`** - 🔥 **LATEST** Real-time web dashboard version (v3.0) - **RECOMMENDED**
- **`smart-helmet-enhanced.ino`** - ✨ Enhanced version with passive buzzer, 3 contacts, battery monitoring, and crash logging (v2.0)
- **`full-code.ino`** - Original working implementation (v1.0)
- **`mpu-test-web-dashboard.ino`** - 🧪 **TESTING TOOL** MPU crash detection testing with web interface - **FOR TESTING ONLY**

### Documentation
- **`README.md`** - This documentation (overview)
- **`MPU_TEST_README.md`** - 🧪 Complete guide for MPU testing tool
- **`WEB_DASHBOARD_README.md`** - 🌐 Complete guide for web dashboard version
- **`LIBRARY_INSTALLATION.md`** - Library installation guide for web dashboard
- **`ENHANCED_VERSION_README.md`** - Complete guide for enhanced version
- **`CRASH_DATA_GUIDE.md`** - Crash data and graph generation guide
- **`SOUND_PATTERNS_GUIDE.md`** - Passive buzzer sound patterns reference
- **`VERSION_COMPARISON.md`** - Feature comparison between versions
- **`IMPLEMENTATION_SUMMARY.md`** - OLED implementation details
- **`OLED_IMPLEMENTATION.md`** - OLED display technical guide
- **`OLED_VISUAL_REFERENCE.md`** - OLED display visual examples

### Utility Scripts
- **`visualize_crashes.py`** - Python script to generate graphs from crash data

### Test/Example Files
- **`mpu-test-web-dashboard.ino`** - 🧪 **MPU Testing Tool** - Web dashboard for crash detection testing
- `crash.ino` - Simple crash detection test
- `gyro-accelerometer.ino` - MPU6050 sensor testing
- `sim800L.ino` - SIM800L SMS testing
- `gps-sim-both-integrated.ino` - GPS + SIM800L integration test

## 🔄 Version Comparison

| Feature | v1.0 (full-code) | v2.0 (enhanced) | v3.0 (web-dashboard) ⭐ | MPU Test Tool 🧪 |
|---------|------------------|-----------------|-------------------------|------------------|
| Crash Detection | ✅ | ✅ | ✅ | ✅ |
| GPS Tracking | ✅ | ✅ | ✅ | ❌ |
| SMS Alerts | ✅ (1 contact) | ✅ (3 contacts) | ✅ (3 contacts) | ❌ |
| Buzzer | Active | Passive w/ tones | Passive w/ tones | ❌ |
| Battery Monitor | ❌ | ✅ | ✅ | ❌ |
| Crash Logging | ❌ | ✅ CSV | ✅ CSV | ✅ CSV |
| OLED Display | ✅ | ✅ | ✅ Enhanced | ❌ |
| Web Dashboard | ❌ | ❌ | ✅ Real-time | ✅ Real-time |
| Live Graphs | ❌ | ❌ | ✅ Yes | ✅ Yes |
| Crash Notes | ❌ | ❌ | ✅ Yes | ❌ |
| Remote Config | ❌ | ❌ | ✅ Yes | ✅ Yes |
| Mobile Access | ❌ | ❌ | ✅ Yes | ✅ Yes |
| Data Download | ❌ | Manual | ✅ One-click | ✅ One-click |
| **Recommended For** | Learning | Daily Use | Production | **Testing** |

**New to Smart Helmet?** Start with `smart-helmet-web-dashboard.ino` (v3.0) - it has all features!

**Want to test crash detection first?** Use `mpu-test-web-dashboard.ino` - minimal setup, pure testing!

## 🤝 Contributing

Contributions are welcome! The enhanced version already includes:
- ✅ Battery monitoring
- ✅ Multiple contact support (3 contacts)
- ✅ Data logging with graphs
- ✅ Advanced sound alerts

Future areas for improvement:
- Voice call integration
- Cloud/web dashboard for location history
- Mobile app integration
- Machine learning for better crash detection
- Low power modes and sleep optimization

## 📝 License

This project is open source and available for educational and personal use.

## 👤 Author

Smart Helmet Project Team

## 🙏 Acknowledgments

- TinyGPSPlus library by Mikal Hart
- ESP32 Arduino Core by Espressif
- Community contributors and testers

## 📞 Support

For issues or questions:
1. Check troubleshooting section above
2. Review serial monitor output for diagnostic info
3. Open an issue on GitHub with:
   - Hardware setup description
   - Serial monitor output
   - Specific problem description

---

**⚠️ IMPORTANT REMINDER**: Change `PHONE_NUMBER` before use! Test thoroughly before relying on system for safety.

**Stay Safe! 🏍️⛑️**
