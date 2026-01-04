# Smart Helmet Deployment & Usage Guide

Complete guide from hardware assembly to daily use.

## 🎯 Overview

This guide walks you through:
1. Hardware assembly and wiring
2. Software installation and configuration
3. Testing and calibration
4. Daily usage
5. Maintenance and troubleshooting

**Time Required**: 2-3 hours for first-time setup

## 📦 What You Need

### Essential Components
- [ ] ESP32 Development Board
- [ ] MPU6050 6-axis sensor (Accelerometer + Gyroscope)
- [ ] Jumper wires (female-to-female)
- [ ] Micro USB cable
- [ ] Computer with Arduino IDE
- [ ] WiFi network (2.4GHz)

### Optional Components (Recommended)
- [ ] Ublox Neo-8M GPS module
- [ ] SIM800L GSM module + SIM card
- [ ] SSD1306 OLED display (128x64)
- [ ] Passive buzzer (3.3V or 5V)
- [ ] Push button
- [ ] 2x 100K resistors (for battery monitor)
- [ ] 7.4V Li-ion battery pack
- [ ] Buck converters (for power regulation)
- [ ] Project box or case

## 🔌 Step 1: Hardware Assembly

### Minimal Setup (ESP32 + MPU6050 Only)

This is enough to get started and use the web dashboard!

**Wiring Table**:

| MPU6050 | ESP32 |
|---------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

**Steps**:
1. Connect VCC (red wire) from MPU6050 to 3.3V on ESP32
2. Connect GND (black wire) from MPU6050 to GND on ESP32
3. Connect SDA (green wire) from MPU6050 to GPIO 21 on ESP32
4. Connect SCL (yellow wire) from MPU6050 to GPIO 22 on ESP32
5. Double-check all connections
6. Connect ESP32 to computer via USB

**Test Connection**:
```cpp
// Upload this simple test first
#include <Wire.h>
#define MPU_ADDR 0x68

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  byte error = Wire.endTransmission();
  if (error == 0) {
    Serial.println("✅ MPU6050 found!");
  } else {
    Serial.println("❌ MPU6050 not found");
  }
}

void loop() {}
```

Expected output: `✅ MPU6050 found!`

### Full Setup (All Components)

**Complete Wiring Diagram**:

```
ESP32 Connections:
==================

MPU6050 (I2C - Sensors):
  VCC  → 3.3V
  GND  → GND
  SDA  → GPIO 21
  SCL  → GPIO 22

GPS Module (UART2):
  VCC  → 5V
  GND  → GND
  TX   → GPIO 16 (ESP32 RX2)
  RX   → GPIO 17 (ESP32 TX2)

SIM800L (UART1):
  VCC  → 4.0V (from buck converter)
  GND  → GND (COMMON GROUND!)
  TX   → GPIO 26 (ESP32 RX1)
  RX   → GPIO 27 (ESP32 TX1)

OLED Display (I2C - shares with MPU6050):
  VCC  → 3.3V
  GND  → GND
  SDA  → GPIO 21 (shared)
  SCL  → GPIO 22 (shared)

Passive Buzzer:
  Positive (+) → GPIO 25
  Negative (-) → GND

Cancel Button:
  One pin → GPIO 33
  Other pin → GND
  (Uses internal pullup, no resistor needed)

Battery Monitor (Voltage Divider):
  Battery+ → 100K resistor → GPIO 34 → 100K resistor → GND
  
  Circuit:
  Battery+ ----[100K]----+---- GPIO 34
                         |
                      [100K]
                         |
                        GND

Power Supply:
  7.4V Battery → Buck 1 (5V) → ESP32 VIN, GPS VCC
              → Buck 2 (4.0V) → SIM800L VCC
  
  All GND connected together (COMMON GROUND!)
```

**Critical Notes**:
1. **SIM800L Power**: MUST be 3.8-4.2V, NOT 3.3V!
2. **Common Ground**: All modules share same GND
3. **Capacitors**: Add 1000µF near SIM800L VCC for stability
4. **I2C Sharing**: MPU6050 and OLED share same SDA/SCL

## 💻 Step 2: Software Setup

### 2.1 Install Arduino IDE

1. Download from: https://www.arduino.cc/en/software
2. Install Arduino IDE 1.8.19 or 2.x
3. Open Arduino IDE

### 2.2 Install ESP32 Board Support

1. **File → Preferences**
2. In "Additional Board Manager URLs", paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Click OK
4. **Tools → Board → Boards Manager**
5. Search "esp32"
6. Install "esp32 by Espressif Systems" (version 2.0.0+)
7. Close Boards Manager

### 2.3 Install Required Libraries

**Method 1: Library Manager (Recommended)**

1. **Sketch → Include Library → Manage Libraries**
2. Install each library:

| Library Name | Author | Purpose |
|--------------|--------|---------|
| WebSockets | Markus Sattler | Real-time communication |
| ArduinoJson | Benoit Blanchon | JSON parsing |
| TinyGPSPlus | Mikal Hart | GPS parsing |
| Adafruit GFX Library | Adafruit | Graphics |
| Adafruit SSD1306 | Adafruit | OLED display |

**Method 2: Quick Install Script**

See [LIBRARY_INSTALLATION.md](LIBRARY_INSTALLATION.md) for detailed guide.

### 2.4 Configure Board Settings

1. **Tools → Board → ESP32 Dev Module**
2. **Tools → Upload Speed → 115200**
3. **Tools → Flash Frequency → 80MHz**
4. **Tools → Partition Scheme → Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)**
5. **Tools → Port → [Select your ESP32's COM port]**

**Finding COM Port**:
- Windows: COM3, COM4, etc.
- Mac: /dev/cu.usbserial-XXXX
- Linux: /dev/ttyUSB0

### 2.5 Configure Code

1. Open `smart-helmet-web-dashboard.ino`

2. **Set WiFi Credentials** (Lines 45-46):
```cpp
const char* WIFI_SSID = "YourWiFiName";       // Change this!
const char* WIFI_PASSWORD = "YourPassword";   // Change this!
```

3. **Set Emergency Contacts** (Lines 85-87):
```cpp
const char PHONE_NUMBER_1[] = "+12125551234";   // Your number
const char PHONE_NUMBER_2[] = "+12125555678";   // Family
const char PHONE_NUMBER_3[] = "+12125559012";   // Friend
```

Include country code! Format: `+[country code][number]`

4. **Adjust Thresholds** (Lines 114-116) - Optional:
```cpp
float ACC_THRESHOLD = 2.5;      // g-force (2.0 = more sensitive)
float GYRO_THRESHOLD = 120.0;   // deg/sec (100 = more sensitive)
int IMPACT_TIME_MS = 30;        // milliseconds (20 = more sensitive)
```

5. Save file (Ctrl+S or Cmd+S)

### 2.6 Upload Code

1. Connect ESP32 to computer via USB
2. Click **Upload** button (→ icon)
3. Wait for compilation (may take 1-2 minutes first time)
4. Wait for upload (shows progress bar)
5. Look for: "Hard resetting via RTS pin..."
6. Success!

**If Upload Fails**:
- Hold BOOT button on ESP32 during upload
- Check correct COM port selected
- Try different USB cable (must support data, not just charging)
- Reduce upload speed to 115200

## 🧪 Step 3: Testing

### 3.1 Initial Test

1. Open **Serial Monitor** (🔍 icon or Ctrl+Shift+M)
2. Set baud rate to **115200**
3. Press ESP32 RESET button

**Expected Output**:
```
========================================
  SMART HELMET WEB DASHBOARD v3.0  
========================================

🔧 Initializing MPU6050...
✅ MPU6050 initialized
🟡 Calibrating gyro (keep helmet still)...
✅ Gyro calibrated
Bias X: -12.34 Y: 5.67 Z: -8.90

🔧 Connecting to WiFi...
.....
✅ WiFi Connected
📱 Dashboard URL: http://192.168.1.105

✅ Web server started
✅ WebSocket started

========================================
   SYSTEM READY - Monitoring...   
========================================
```

**If Errors Occur**:
- "MPU6050 not responding" → Check wiring
- "WiFi connection failed" → Check credentials
- "SIM800L not ready" → Normal if not installed

### 3.2 Access Dashboard

1. **Note the IP address** from Serial Monitor
   Example: `http://192.168.1.105`

2. On your **phone or laptop**:
   - Ensure connected to **same WiFi network**
   - Open web browser (Chrome recommended)
   - Go to: `http://[YOUR-ESP32-IP]`

3. Dashboard should load in 1-2 seconds!

**What You'll See**:
- ● CONNECTED (green dot top-right)
- System status (GPS, Battery, Crashes)
- Live graphs (blue/purple lines)
- Configuration section
- Empty crash history

### 3.3 Test Sensors

1. **Check Graphs Are Updating**:
   - Pick up ESP32
   - Tilt it slowly
   - Watch graphs respond
   - Should update 5 times per second

2. **Test Acceleration**:
   - Move ESP32 up and down
   - Watch acceleration graph spike
   - Normal movement: 0.9-1.2g
   - Quick movement: 2.0-3.0g

3. **Test Gyroscope**:
   - Rotate ESP32 slowly
   - Watch gyroscope graph spike
   - Slow rotation: 10-50°/s
   - Fast rotation: 100-200°/s

### 3.4 Test Crash Detection

**Safe Test Method**:

1. Set conservative thresholds via dashboard:
   - Acc: 2.5g
   - Gyro: 120°/s
   - Impact: 30ms
   - Click "Update Thresholds"

2. Drop ESP32 onto **soft cushion/pillow** from 30cm height

3. **Expected Behavior**:
   - Graphs spike RED
   - Serial Monitor: "🚨🚨🚨 CRASH DETECTED 🚨🚨🚨"
   - Buzzer sounds (if connected)
   - OLED shows alert (if connected)
   - New crash appears in history

4. Check Crash History:
   - Scroll to bottom of dashboard
   - Should see Crash #1
   - Contains timestamp, sensor values
   - Click "Add note", type "Test crash"
   - Click "Save Note"

5. Download CSV:
   - Click "Download CSV"
   - File downloads: crashes.csv
   - Open in Excel or text editor
   - Verify data is correct

**If No Crash Detected**:
- Lower thresholds (Acc: 2.0g, Gyro: 100°/s)
- Try harder impact
- Check MPU6050 is secure (not loose)

**If Too Many False Alarms**:
- Increase thresholds (Acc: 3.0g, Gyro: 150°/s)
- Increase impact time (50ms)
- Mount MPU6050 more securely

## 🏍️ Step 4: Helmet Installation

### 4.1 Component Placement

**Inside Helmet**:
- **ESP32**: Top center (foam padding)
- **MPU6050**: Secure to ESP32 (double-sided tape)
- **OLED**: Front visible area
- **GPS**: Top, antenna facing sky
- **Battery**: Back of helmet
- **Buzzer**: Side near ear

**Outside Helmet**:
- **SIM800L**: In waterproof case if needed
- **Antennas**: GPS and GSM antennas outside

### 4.2 Mounting Tips

1. **Use foam padding** to cushion electronics
2. **Secure with velcro** for easy removal
3. **Keep MPU6050 stable** - critical for accuracy
4. **Protect from moisture** - use waterproof cases
5. **Easy access to power** - for charging
6. **Test before final mounting** - ensure everything works

### 4.3 Cable Management

- Use cable ties to bundle wires
- Route along helmet interior
- Avoid pressure points
- Leave service loops for maintenance
- Label wires for easy identification

## 📱 Step 5: Daily Usage

### Before Each Ride

1. **Power On**: Connect battery or USB power
2. **Wait 10 seconds**: For system to boot
3. **Check OLED** (if installed): Should show "Ready"
4. **Open Dashboard**:
   - Phone: Open bookmarked URL
   - Or type: `http://192.168.1.XXX`
5. **Verify Connection**: Green dot = connected
6. **Check GPS**: "Connected" or "Searching..."
7. **Check Battery**: Should show percentage

### During Ride

**Option 1: Active Monitoring**
- Mount phone on bike
- Dashboard shows real-time data
- Glance at graphs while stopped

**Option 2: Passive Monitoring**
- Keep phone in pocket
- System runs independently
- Check after ride

### After Ride

1. **Review Crashes**:
   - Open dashboard
   - Scroll to Crash History
   - Click "Refresh History"

2. **Add Notes**:
   - For each crash, add context
   - Example: "Speed bump, false alarm"
   - Click "Save Note"

3. **Check Locations**:
   - Click "View on Map" for each crash
   - Identify dangerous spots

4. **Download Data** (Weekly):
   - Click "Download CSV"
   - Archive data
   - Analyze trends

5. **Adjust Settings** (As Needed):
   - Too sensitive? Increase thresholds
   - Not sensitive? Decrease thresholds

### Weekly Maintenance

- [ ] Charge battery fully
- [ ] Check all connections
- [ ] Clean components (dust off)
- [ ] Test crash detection
- [ ] Download and backup crash data
- [ ] Review and clear old crashes if needed

### Monthly Maintenance

- [ ] Full system test
- [ ] Recalibrate gyroscope (reboot)
- [ ] Check wire integrity
- [ ] Update firmware if available
- [ ] Replace batteries if low performance
- [ ] Check SIM card balance (for SMS)

## 🔧 Troubleshooting

### Dashboard Not Loading

**Problem**: Can't access dashboard on phone

**Check**:
1. Phone on same WiFi? → Connect to helmet's WiFi network
2. Correct IP address? → Check Serial Monitor for current IP
3. ESP32 powered on? → Check OLED or LED lights
4. WiFi connected? → Serial Monitor shows "WiFi Connected"

**Solutions**:
- Reboot ESP32 (press RESET button)
- Restart WiFi router
- Forget WiFi network on phone and reconnect
- Try different browser
- Check firewall settings

### WebSocket Disconnected

**Problem**: Red "DISCONNECTED" dot

**Solutions**:
- Wait 3 seconds for auto-reconnect
- Refresh browser page (F5)
- Check ESP32 is powered
- Check Serial Monitor for errors
- Reboot ESP32 if persists

### False Crash Alerts

**Problem**: Crashes detected during normal riding

**Cause**: Thresholds too sensitive for your riding style

**Solutions**:
1. Increase Acceleration threshold: 2.5 → 3.0g
2. Increase Gyroscope threshold: 120 → 150°/s
3. Increase Impact time: 30 → 50ms
4. Test on safe road
5. Repeat until comfortable

**Finding Right Thresholds**:
- Rough roads: Acc 3.0g, Gyro 150°/s
- Smooth roads: Acc 2.5g, Gyro 120°/s
- Off-road: Acc 3.5g, Gyro 180°/s

### Real Crashes Not Detected

**Problem**: Had accident but no alert

**Cause**: Thresholds too high

**Solutions**:
1. Lower Acceleration: 2.5 → 2.0g
2. Lower Gyroscope: 120 → 100°/s
3. Lower Impact time: 30 → 20ms
4. Test with controlled drops
5. Ensure MPU6050 is mounted securely

### Battery Draining Fast

**Problem**: Battery doesn't last long

**Solutions**:
- Use larger capacity battery (e.g., 2000mAh)
- Disable OLED if not needed
- Reduce WiFi transmission power (in code)
- Turn off when not riding
- Check for short circuits

### GPS Not Getting Fix

**Problem**: GPS shows "Searching..." always

**Solutions**:
- Move to open area (outdoors)
- Wait 2-5 minutes for first fix
- Check GPS antenna is connected
- Verify GPS module power (5V)
- Check TX/RX not swapped
- Try different GPS module

## 📊 Understanding the Data

### Acceleration Graph

**Normal Values**:
- Stationary: ~1.0g (gravity)
- Gentle movement: 1.0-1.5g
- Normal bumps: 1.5-2.0g
- Large bumps: 2.0-2.5g
- **Crash threshold: 2.5g+**

**Interpretation**:
- Smooth line = smooth riding
- Small spikes = normal bumps
- Large red spike = potential crash
- Continuous high values = rough road

### Gyroscope Graph

**Normal Values**:
- Stationary: 0-5°/s
- Gentle turns: 10-30°/s
- Normal turns: 30-80°/s
- Sharp turns: 80-120°/s
- **Crash threshold: 120°/s+**

**Interpretation**:
- Flat line = straight riding
- Regular spikes = turns
- Large red spike = rapid rotation (crash)
- Erratic = unstable riding

### Crash Events (Red Markers)

**What Red Means**:
- Both sensors exceeded thresholds
- Sustained for required time (30ms default)
- High confidence crash event
- System triggered alerts

**Reviewing Crashes**:
1. Check timestamp - when did it happen?
2. Check magnitude - how severe?
3. Check location - where (if GPS available)?
4. Add note - what actually happened?
5. Adjust thresholds if false alarm

## 🎯 Best Practices

### Threshold Tuning

**Conservative (Few False Alarms)**:
- Acc: 3.0g
- Gyro: 150°/s
- Impact: 50ms
- Good for: Rough roads, off-road

**Balanced (Recommended)**:
- Acc: 2.5g
- Gyro: 120°/s
- Impact: 30ms
- Good for: Normal city riding

**Sensitive (Detect Minor Incidents)**:
- Acc: 2.0g
- Gyro: 100°/s
- Impact: 20ms
- Good for: Smooth roads, highways

**Test Process**:
1. Start with Balanced settings
2. Ride for one week
3. Review crash history
4. If too many false alarms → increase thresholds
5. If concerned about missing crashes → decrease
6. Repeat until satisfied

### Data Management

**Keep History Clean**:
- Add notes to distinguish real vs false crashes
- Clear obvious false alarms weekly
- Download CSV monthly for long-term storage
- Keep last 50-100 crashes on device

**Analyzing Trends**:
- Use Python script to generate graphs
- Look for patterns (time, location, conditions)
- Identify dangerous areas
- Share data with authorities if needed

### Battery Life

**Typical Battery Life** (2000mAh):
- ESP32: ~100mA
- MPU6050: ~5mA
- GPS: ~30mA
- SIM800L (standby): ~10mA
- OLED: ~20mA
- **Total: ~165mA**
- **Runtime: ~12 hours**

**Extend Battery Life**:
- Turn off when not riding
- Use larger battery
- Disable OLED (saves 20mA)
- Reduce GPS update rate
- Use power-saving modes in code

## 📞 Support

### Getting Help

1. **Read Documentation**:
   - [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md)
   - [QUICK_START.md](QUICK_START.md)
   - This guide

2. **Check Serial Monitor**:
   - Shows detailed error messages
   - Indicates what's not working
   - Copy output for support requests

3. **Browser Console**:
   - Press F12 in browser
   - Check Console tab
   - Look for errors (red text)

4. **Open GitHub Issue**:
   - Describe problem clearly
   - Include Serial Monitor output
   - Include Browser Console output
   - List your hardware setup
   - Mention what you've tried

### Emergency Contacts Configuration

Remember to set REAL emergency contacts:

```cpp
const char PHONE_NUMBER_1[] = "+12125551234";   // YOU
const char PHONE_NUMBER_2[] = "+12125555678";   // FAMILY
const char PHONE_NUMBER_3[] = "+12125559012";   // FRIEND
```

**Test SMS Sending** before relying on it:
1. Configure one real number
2. Trigger test crash (cushion drop)
3. Verify SMS received
4. Check message format
5. Test Google Maps link

## 🎓 Advanced Usage

### Port Forwarding (Remote Access)

To access dashboard from anywhere:

1. **Configure Router**:
   - Forward external port 8080 → ESP32 IP:80
   - Forward external port 8081 → ESP32 IP:81

2. **Find Public IP**:
   - Visit whatismyip.com
   - Note your public IP

3. **Access Remotely**:
   - URL: `http://[public-ip]:8080`
   - Share with family for monitoring

4. **Security**:
   - Consider adding password protection
   - Use VPN for better security
   - Monitor access logs

### Custom Alerts

Add custom alert sounds or vibrations:

```cpp
// In Arduino code, after crash detection:
if (crashDetected) {
  // Your custom alert code here
  // Example: Flash LED
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
}
```

### Integration with Other Systems

**Send to Cloud**:
```cpp
// Add after logCrashData():
HTTPClient http;
http.begin("https://your-api.com/crash");
http.POST(crashJSON);
```

**Trigger Camera**:
```cpp
// After crash detected:
digitalWrite(CAMERA_TRIGGER_PIN, HIGH);
delay(100);
digitalWrite(CAMERA_TRIGGER_PIN, LOW);
```

## ✅ Deployment Checklist

Before first real-world use:

### Hardware
- [ ] All components connected correctly
- [ ] Wiring secured and insulated
- [ ] MPU6050 firmly mounted
- [ ] Battery fully charged
- [ ] SIM card inserted and active
- [ ] All antennas connected
- [ ] Helmet comfortable with electronics

### Software
- [ ] WiFi credentials correct
- [ ] Emergency contacts configured
- [ ] Thresholds adjusted for your riding
- [ ] Dashboard accessible from phone
- [ ] WebSocket connecting properly
- [ ] Graphs updating in real-time
- [ ] SMS test successful

### Testing
- [ ] Crash detection works (cushion test)
- [ ] GPS getting fix outdoors
- [ ] Battery lasts expected duration
- [ ] OLED display working
- [ ] Buzzer audible
- [ ] Dashboard bookmarked on phone
- [ ] Backup contacts notified of system

### Documentation
- [ ] Emergency contacts know about system
- [ ] Family has dashboard URL
- [ ] Insurance company notified (if relevant)
- [ ] Maintenance schedule set

## 🎉 You're Ready!

Congratulations! Your Smart Helmet is deployed and ready for use.

**Remember**:
- System enhances safety, doesn't replace it
- Wear proper safety gear always
- Ride defensively
- Regular maintenance is crucial
- Test monthly to ensure functionality

**Stay Safe! Ride Smart! 🏍️⛑️📱**

---

**Questions?** Refer to comprehensive guides:
- [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md) - Complete web dashboard guide
- [QUICK_START.md](QUICK_START.md) - Quick 10-minute setup
- [LIBRARY_INSTALLATION.md](LIBRARY_INSTALLATION.md) - Library setup help

**Issues?** Open a GitHub issue with details!
