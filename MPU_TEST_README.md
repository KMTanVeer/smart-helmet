# MPU Test & Recording System - Web Dashboard

## 🎯 Purpose

This is a **simplified testing tool** designed specifically to:
- Test and validate MPU6050 crash detection logic
- Record real-time accelerometer and gyroscope data
- Visualize crashes in RED on live graphs
- Download recorded data as CSV for analysis
- No GPS, no SMS, no emergency features - **TESTING ONLY**

## ✨ Features

### Real-Time Monitoring
- ✅ Live acceleration data (g-force)
- ✅ Live gyroscope data (degrees/second)
- ✅ Real-time crash detection
- ✅ Crashes highlighted in **RED** on graphs
- ✅ Crash events marked with "CRASH DETECTED"

### Web Dashboard
- 📊 Live graphs with Chart.js
- 📱 Mobile-friendly responsive design
- 🔴 Red markers on crash points
- 💾 Download all data as CSV
- ⚙️ Adjustable thresholds via web interface
- 🔄 WebSocket for instant updates (5 Hz)

### Data Recording
- 📝 Records up to 1000 data points
- 💾 Stores timestamp, acceleration, gyroscope, crash status
- 📥 One-click CSV download to phone/computer
- 🗑️ Clear all data with one button

## 🔧 Hardware Requirements

### Minimal Setup - Only 2 Components!

1. **ESP32 Development Board**
2. **MPU6050** - 6-axis Gyroscope & Accelerometer

That's it! No GPS, no SIM800L, no other sensors needed.

### Wiring Diagram

```
ESP32 PIN CONNECTIONS:
======================

MPU6050:
├── VCC  -> 3.3V
├── GND  -> GND
├── SDA  -> GPIO 21
└── SCL  -> GPIO 22

Power:
└── ESP32 via USB (for testing)
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
   
   Go to Sketch → Include Library → Manage Libraries, then install:
   
   - **WebSockets** by Markus Sattler
   - **ArduinoJson** by Benoit Blanchon (v6.x)
   - **Wire** (built-in, no installation needed)

3. **Board Configuration**:
   - Board: "ESP32 Dev Module"
   - Upload Speed: 115200
   - Flash Frequency: 80MHz
   - Port: Select your ESP32's COM port

## 🚀 Quick Start Guide

### Step 1: Configure WiFi

Open `mpu-test-web-dashboard.ino` and change these lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### Step 2: Upload Code

1. Connect ESP32 to computer via USB
2. Select correct COM port in Arduino IDE
3. Click Upload button
4. Wait for upload to complete

### Step 3: Get IP Address

1. Open Serial Monitor (115200 baud)
2. Wait for system to initialize
3. Look for this line:
   ```
   📱 IP Address: 192.168.1.xxx
   🌐 Open browser to: http://192.168.1.xxx
   ```
4. Note down the IP address

### Step 4: Open Web Dashboard

1. On your phone or computer (connected to same WiFi)
2. Open web browser
3. Go to: `http://[ESP32-IP-ADDRESS]`
4. You should see the dashboard!

## 📱 Using the Dashboard

### Main Interface

The dashboard shows:

1. **Connection Status** (top-right corner)
   - Green "CONNECTED" = WebSocket active
   - Red "DISCONNECTED" = Connection lost

2. **Real-Time Values** (top section)
   - Current acceleration (g)
   - Current gyroscope (°/s)
   - Current thresholds

3. **Configuration Section**
   - Adjust crash detection thresholds
   - Update settings in real-time
   - Get current settings

4. **Live Graphs**
   - Acceleration graph (blue line)
   - Gyroscope graph (green line)
   - **Crash points marked in RED** 🔴

5. **Crash History**
   - List of all detected crashes
   - Timestamp, acceleration, gyroscope values
   - Red background for each crash event

6. **Data Management**
   - Download CSV button
   - Clear all data button
   - Refresh history button

### Testing Crash Detection

1. **View Normal Data**
   - Keep helmet still
   - Should see ~1.0g acceleration (gravity)
   - Low gyroscope values

2. **Simulate Crash**
   - Shake the helmet vigorously
   - Or drop it on a padded surface
   - Watch for RED markers on graphs
   - "CRASH DETECTED!" appears in status

3. **Adjust Sensitivity**
   - Too sensitive? Increase thresholds
   - Not sensitive enough? Decrease thresholds
   - Click "Update Thresholds" to apply

4. **Download Data**
   - Click "Download CSV"
   - Save to phone/computer
   - Open in Excel/Google Sheets
   - Analyze crash patterns

## 📊 CSV Data Format

When you download CSV, you get:

```csv
Timestamp (ms),Acceleration (g),Gyroscope (deg/s),Crash Detected
1234,1.02,3.45,NO
1434,0.98,2.31,NO
1634,3.87,145.67,YES
1834,1.05,4.12,NO
```

Each row contains:
- **Timestamp**: Milliseconds since system start
- **Acceleration**: Total g-force magnitude
- **Gyroscope**: Total rotation speed
- **Crash Detected**: YES or NO

## ⚙️ Crash Detection Settings

### Default Thresholds

```cpp
ACC_THRESHOLD = 2.5;      // g-force (2.5g = significant impact)
GYRO_THRESHOLD = 120.0;   // degrees/second (rapid rotation)
IMPACT_TIME_MS = 30;      // milliseconds (sustained impact)
```

### How It Works

A crash is detected when **ALL** conditions are met:
1. Acceleration > ACC_THRESHOLD
2. Gyroscope > GYRO_THRESHOLD
3. Both conditions sustained for > IMPACT_TIME_MS

### Tuning Guidelines

**For motorcycle/bike crashes:**
- ACC_THRESHOLD: 2.0 - 3.0 g
- GYRO_THRESHOLD: 100 - 150 °/s

**For high-sensitivity testing:**
- ACC_THRESHOLD: 1.5 - 2.0 g
- GYRO_THRESHOLD: 80 - 100 °/s

**For low false-positive rate:**
- ACC_THRESHOLD: 3.0 - 4.0 g
- GYRO_THRESHOLD: 150 - 200 °/s

## 🔍 Troubleshooting

### WiFi Issues

**Problem**: "WiFi connection failed"
- Check SSID and password are correct
- Ensure WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Move ESP32 closer to router

**Problem**: Can't access dashboard
- Verify ESP32 and phone/computer on same WiFi network
- Check IP address in Serial Monitor
- Try different browser (Chrome recommended)

### MPU6050 Issues

**Problem**: Erratic readings
- Check wiring connections (SDA/SCL)
- Ensure MPU6050 is powered by 3.3V
- Keep helmet still during calibration

**Problem**: No data on graphs
- Check WebSocket connection (top-right status)
- Refresh browser page
- Check Serial Monitor for errors

### WebSocket Issues

**Problem**: Dashboard shows "DISCONNECTED"
- Wait 3 seconds for auto-reconnect
- Refresh browser page
- Check Serial Monitor for WebSocket errors

## 📈 Serial Monitor Output

During normal operation:

```
========================================
   MPU TEST & RECORDING SYSTEM         
========================================

🔧 Initializing MPU6050...
✅ MPU6050 initialized
🟡 Calibrating gyro (keep still for 2 seconds)...
✅ Gyro calibrated
Bias X: -12.34 Y: 5.67 Z: -8.90
🔧 Connecting to WiFi...
........
✅ WiFi connected!
📱 IP Address: 192.168.1.100
🌐 Open browser to: http://192.168.1.100
✅ Web server started
✅ WebSocket server started

========================================
   SYSTEM READY - TESTING MODE         
========================================

ACC(g): 1.02 | GYRO(dps): 3.4
ACC(g): 0.98 | GYRO(dps): 2.1
ACC(g): 3.45 | GYRO(dps): 156.7 [IMPACT DETECTED]
🚨🚨🚨 CRASH DETECTED 🚨🚨🚨 (Count: 1)
ACC(g): 1.01 | GYRO(dps): 3.8
```

## 📊 Data Analysis Tips

After downloading CSV data:

1. **Import to Excel/Google Sheets**
   - Open CSV file
   - Create line graphs for acceleration and gyroscope
   - Filter rows where "Crash Detected" = YES

2. **Identify Patterns**
   - What acceleration values trigger crashes?
   - What gyroscope values are typical?
   - How long do crash events last?

3. **Optimize Thresholds**
   - If too many false positives: increase thresholds
   - If missing real crashes: decrease thresholds
   - Test multiple scenarios (riding, parking, dropping)

4. **Document Results**
   - Note crash scenarios that work well
   - Note false positive scenarios
   - Use this data to fine-tune main helmet system

## 🎯 Testing Scenarios

### Recommended Tests

1. **Baseline Test**
   - Keep helmet on table
   - Record for 1 minute
   - Should see stable ~1.0g, low gyro

2. **Vibration Test**
   - Shake helmet gently
   - Should NOT trigger crash
   - If it does, increase thresholds

3. **Drop Test**
   - Drop helmet on padded surface
   - Should trigger crash detection
   - If not, decrease thresholds

4. **Rotation Test**
   - Rotate helmet quickly
   - Check gyroscope values
   - Verify threshold is appropriate

5. **Impact Test**
   - Hit helmet on soft surface
   - Should trigger both acc and gyro
   - Verify crash marked in RED

## 💡 Key Differences from Main System

This testing code **DOES NOT HAVE**:
- ❌ GPS module
- ❌ SIM800L GSM module
- ❌ Emergency SMS sending
- ❌ Emergency contacts
- ❌ Buzzer alerts
- ❌ OLED display
- ❌ Battery monitoring
- ❌ Cancel button

This testing code **ONLY HAS**:
- ✅ MPU6050 sensor
- ✅ Crash detection logic
- ✅ Web dashboard
- ✅ Data recording
- ✅ CSV download

**Purpose**: Test crash detection algorithm before deploying full system!

## 🔄 Next Steps

After testing and validating crash detection:

1. **Analyze Results**
   - Review downloaded CSV data
   - Identify optimal thresholds
   - Document crash patterns

2. **Transfer Settings**
   - Use tested thresholds in main system
   - Update `full-code.ino` or `smart-helmet-web-dashboard.ino`
   - Set `ACC_THRESHOLD` and `GYRO_THRESHOLD`

3. **Deploy Full System**
   - Add GPS, SIM800L, other components
   - Use validated crash detection logic
   - Confident in crash detection accuracy!

## 📝 Files

- **mpu-test-web-dashboard.ino** - Main testing code
- **MPU_TEST_README.md** - This documentation

## 🤝 Support

For issues:
1. Check Serial Monitor for diagnostic info
2. Verify wiring connections
3. Ensure WiFi credentials are correct
4. Check library versions are compatible

## ⚠️ Important Notes

- This is a **TESTING TOOL ONLY**
- Not for actual crash detection deployment
- No emergency features included
- Use to validate crash logic before full deployment
- Always wear proper safety gear when riding

---

**Happy Testing! 🧪🛡️**
