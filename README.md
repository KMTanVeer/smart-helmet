# Smart Helmet - Accident Detection & Emergency Alert System

An intelligent motorcycle/bike helmet safety system that automatically detects accidents using motion sensors and sends emergency SMS alerts with GPS location to designated contacts.

## 🎯 Features

- **Automatic Accident Detection**: Uses MPU6050 6-axis gyroscope and accelerometer to detect crashes
- **GPS Location Tracking**: Ublox Neo-8M GPS module provides accurate location coordinates
- **Emergency SMS Alerts**: SIM800L GSM module sends SMS with Google Maps link to emergency contacts
- **Audio Alert**: Buzzer activates when accident is detected
- **False Alarm Prevention**: Manual button to cancel false positives
- **Professional SIM800L Handling**: Robust error checking and network status monitoring
- **Modular Code Design**: Well-organized, documented, and easy to customize

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
   const char PHONE_NUMBER[] = "+8801747213525";  // Change this!
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

- `full-code.ino` - Main complete system implementation
- `crash.ino` - Simple crash detection test
- `gyro-accelerometer.ino` - MPU6050 sensor testing
- `sim800L.ino` - SIM800L SMS testing
- `gps-sim-both-integrated.ino` - GPS + SIM800L integration test
- `README.md` - This documentation file

## 🤝 Contributing

Contributions are welcome! Areas for improvement:
- Battery monitoring and low power modes
- Multiple contact support
- Voice call integration
- SD card logging
- Web dashboard for location history
- Machine learning for better crash detection

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
