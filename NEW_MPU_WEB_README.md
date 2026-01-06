# New MPU Test Web Dashboard - Dhaka Timezone Edition

## 🎯 Purpose

This is an **enhanced MPU testing tool** with real timezone support for Dhaka, Bangladesh. It builds upon the original `mpu-test-web-dashboard.ino` with the following improvements:

- Real-time clock synchronization with NTP (Network Time Protocol)
- Displays all timestamps in **Dhaka, Bangladesh timezone (GMT+6)**
- Enhanced crash visualization with **RED markers** on graphs
- Better crash history tracking with human-readable timestamps
- All features from the original dashboard plus timezone support

## ✨ Key Features

### Real-Time Monitoring
- ✅ Live acceleration data (g-force)
- ✅ Live gyroscope data (degrees/second)
- ✅ Real-time crash detection
- ✅ Crashes highlighted in **RED** with large markers on graphs
- ✅ Crash events marked with "CRASH DETECTED"
- 🆕 **Real Dhaka timezone display** (GMT+6)
- 🆕 **Synchronized time with NTP servers**

### Web Dashboard
- 📊 Live graphs with Chart.js
- 📱 Mobile-friendly responsive design
- 🔴 Red markers (large radius) on crash points
- 💾 Download all data as CSV with Dhaka time
- ⚙️ Adjustable thresholds via web interface
- 🔄 WebSocket for instant updates (5 Hz)
- 🇧🇩 Dhaka timezone badge on dashboard
- 🕐 Live current time display

### Data Recording
- 📝 Records up to 1000 data points
- 💾 Stores timestamp, Dhaka time, acceleration, gyroscope, crash status
- 📥 One-click CSV download with Dhaka time column
- 🗑️ Clear all data with one button

## 🔧 Hardware Requirements

### Minimal Setup - Only 2 Components!

1. **ESP32 Development Board** (with WiFi)
2. **MPU6050** - 6-axis Gyroscope & Accelerometer

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
   - **NTPClient** by Fabrice Weinberg
   - **Wire** (built-in, no installation needed)

3. **Board Configuration**:
   - Board: "ESP32 Dev Module"
   - Upload Speed: 115200
   - Flash Frequency: 80MHz
   - Port: Select your ESP32's COM port

## 🚀 Quick Start Guide

### Step 1: Configure WiFi

Open `new-mpu-web.ino` and change these lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### Step 2: Upload Code

1. Connect ESP32 to computer via USB
2. Select correct COM port in Arduino IDE
3. Click Upload button
4. Wait for upload to complete

### Step 3: Get IP Address and Check Time

1. Open Serial Monitor (115200 baud)
2. Wait for system to initialize
3. Look for these lines:
   ```
   ✅ WiFi connected!
   📱 IP Address: 192.168.1.xxx
   🌐 Open browser to: http://192.168.1.xxx
   ✅ Time synchronized!
   🇧🇩 Dhaka Time: 2024-01-06 19:30:45
   ```
4. Note down the IP address

### Step 4: Open Web Dashboard

1. On your phone or computer (connected to same WiFi)
2. Open web browser
3. Go to: `http://[ESP32-IP-ADDRESS]`
4. You should see the dashboard with Dhaka timezone!

## 📱 Using the Dashboard

### Main Interface

The dashboard shows:

1. **Connection Status** (top-right corner)
   - Green "CONNECTED" = WebSocket active
   - Red "DISCONNECTED" = Connection lost

2. **Header Section**
   - Title and description
   - 🇧🇩 Dhaka, Bangladesh (GMT+6) badge
   - 🕐 Live current time in Dhaka timezone
   - Status summary (monitoring, crash count, data points)

3. **Real-Time Values**
   - Current acceleration (g)
   - Current gyroscope (°/s)
   - Current thresholds

4. **Configuration Section**
   - Adjust crash detection thresholds
   - Update settings in real-time
   - Get current settings

5. **Live Graphs**
   - Acceleration graph (blue line)
   - Gyroscope graph (green line)
   - **Crash points marked with large RED dots** 🔴
   - Hover over crash points to see "🚨 CRASH DETECTED"

6. **Crash History**
   - List of all detected crashes
   - **Timestamp in Dhaka timezone**
   - Acceleration and gyroscope values
   - Red background for each crash event

7. **Data Management**
   - Download CSV with Dhaka time
   - Clear all data button
   - Refresh history button

## 🕐 Timezone Features

### How It Works

The system uses NTP (Network Time Protocol) to synchronize with internet time servers and automatically applies the Dhaka timezone offset (GMT+6).

**Key Points:**
- Connects to `pool.ntp.org` NTP server
- Applies +6 hours offset (21600 seconds)
- Updates time every 60 seconds
- Displays time in 24-hour format
- Format: `YYYY-MM-DD HH:MM:SS`

### Time Display Locations

1. **Dashboard Header**: Live current time (updates every second)
2. **Serial Monitor**: Timestamps on each data line
3. **Crash History**: Dhaka time for each crash event
4. **CSV Download**: Dedicated "Dhaka Time" column

### Internet Connection Required

⚠️ **Important**: The ESP32 must have internet access through your WiFi to sync time with NTP servers. If time sync fails, the system will:
- Show "Time not synced" or "Syncing time..."
- Continue operating normally
- Retry time sync in the background
- Use milliseconds timestamp as fallback

## 📊 CSV Data Format

When you download CSV, you get:

```csv
Timestamp (ms),Dhaka Time,Acceleration (g),Gyroscope (deg/s),Crash Detected
1234,2024-01-06 19:30:45,1.02,3.45,NO
1434,2024-01-06 19:30:46,0.98,2.31,NO
1634,2024-01-06 19:30:47,3.87,145.67,YES
1834,2024-01-06 19:30:48,1.05,4.12,NO
```

Each row contains:
- **Timestamp**: Milliseconds since system start
- **Dhaka Time**: Human-readable time in Dhaka timezone
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

When a crash is detected:
- 🔴 **Large RED markers** appear on both graphs
- Status changes to "🚨 CRASH DETECTED!"
- Crash added to history with Dhaka time
- Serial monitor shows crash with timestamp
- WebSocket broadcasts crash event

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

### Time Sync Issues

**Problem**: "Time not synced" displayed
- Check WiFi has internet access
- Verify firewall isn't blocking NTP (UDP port 123)
- Wait 10-20 seconds for initial sync
- Check Serial Monitor for time sync status

**Problem**: Wrong time displayed
- Verify timezone offset is correct (21600 for Dhaka)
- Check system is actually getting NTP updates
- Restart ESP32 to force fresh time sync

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
   MPU TEST WITH DHAKA TIMEZONE         
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
🕐 Synchronizing time with NTP server...
.....
✅ Time synchronized!
🇧🇩 Dhaka Time: 2024-01-06 19:30:45
✅ Web server started
✅ WebSocket server started

========================================
   SYSTEM READY - TESTING MODE         
   Timezone: Dhaka, Bangladesh (GMT+6)  
========================================

[2024-01-06 19:30:45] ACC(g): 1.02 | GYRO(dps): 3.4
[2024-01-06 19:30:46] ACC(g): 0.98 | GYRO(dps): 2.1
[2024-01-06 19:30:47] ACC(g): 3.45 | GYRO(dps): 156.7 [IMPACT DETECTED]
🚨🚨🚨 CRASH DETECTED 🚨🚨🚨 (Count: 1) at 2024-01-06 19:30:47
[2024-01-06 19:30:48] ACC(g): 1.01 | GYRO(dps): 3.8
```

## 🎯 Testing Scenarios

### Recommended Tests

1. **Baseline Test**
   - Keep helmet on table
   - Record for 1 minute
   - Should see stable ~1.0g, low gyro
   - Verify Dhaka time is correct

2. **Vibration Test**
   - Shake helmet gently
   - Should NOT trigger crash
   - If it does, increase thresholds

3. **Drop Test**
   - Drop helmet on padded surface
   - Should trigger crash detection
   - Check RED markers appear on graphs
   - Verify Dhaka time is recorded

4. **Rotation Test**
   - Rotate helmet quickly
   - Check gyroscope values
   - Verify threshold is appropriate

5. **Impact Test**
   - Hit helmet on soft surface
   - Should trigger both acc and gyro
   - Verify crash marked in RED on both graphs
   - Check crash history shows Dhaka time

## 💡 Key Improvements Over Original

This version **ADDS**:
- ✅ Real timezone support (Dhaka, Bangladesh GMT+6)
- ✅ NTP time synchronization
- ✅ Human-readable timestamps in crash history
- ✅ Dhaka time column in CSV downloads
- ✅ Live current time display
- ✅ Timezone badge on dashboard
- ✅ Enhanced crash visualization with larger RED markers
- ✅ Time information in serial monitor

This version **KEEPS**:
- ✅ All original MPU6050 functionality
- ✅ Crash detection logic
- ✅ Web dashboard with graphs
- ✅ WebSocket real-time updates
- ✅ Configurable thresholds
- ✅ CSV data download
- ✅ 1000 data point storage

## 🔄 Comparison with Original

| Feature | mpu-test-web-dashboard.ino | new-mpu-web.ino |
|---------|---------------------------|-----------------|
| MPU6050 Sensor | ✅ | ✅ |
| Web Dashboard | ✅ | ✅ |
| Crash Detection | ✅ | ✅ |
| RED Crash Markers | ✅ | ✅ Enhanced |
| CSV Download | ✅ | ✅ with Dhaka time |
| Timezone Support | ❌ | ✅ Dhaka GMT+6 |
| NTP Sync | ❌ | ✅ |
| Human-readable Time | ❌ | ✅ |
| Live Clock Display | ❌ | ✅ |

## 📝 Code Structure

### Main Components

1. **NTPClient**: Handles time synchronization
   - Connects to pool.ntp.org
   - Applies GMT+6 offset
   - Updates every 60 seconds

2. **DataPoint Structure**: Enhanced with time string
   ```cpp
   struct DataPoint {
     unsigned long timestamp;
     float accMag;
     float gyroMag;
     bool isCrash;
     String timeString;  // Dhaka time
   };
   ```

3. **Web Dashboard**: Enhanced HTML/JavaScript
   - Timezone badge
   - Live time display
   - RED crash markers with tooltips
   - Dhaka time in crash history

4. **API Endpoints**:
   - `/` - Main dashboard
   - `/getCurrentTime` - Get current Dhaka time
   - `/getCrashHistory` - Get crashes with Dhaka time
   - `/downloadCSV` - Download with Dhaka time column
   - `/updateThresholds` - Update crash thresholds
   - `/getSettings` - Get current settings
   - `/clearData` - Clear all recorded data

## 🔐 Security Notes

- Uses WPA/WPA2 WiFi security
- No authentication on web dashboard (for testing only)
- NTP uses UDP (no encryption)
- For production use, add authentication

## 📦 Dependencies

```
ESP32 Core >= 2.0.0
WebSockets by Markus Sattler >= 2.3.0
ArduinoJson by Benoit Blanchon >= 6.0.0
NTPClient by Fabrice Weinberg >= 3.2.0
```

## 🤝 Support

For issues:
1. Check Serial Monitor for diagnostic info
2. Verify wiring connections
3. Ensure WiFi credentials are correct
4. Check library versions are compatible
5. Verify internet access for NTP

## ⚠️ Important Notes

- This is a **TESTING TOOL ONLY**
- Not for actual crash detection deployment
- No emergency features included
- Requires internet for time sync
- Use to validate crash logic before full deployment
- Always wear proper safety gear when riding

## 🎓 Learning Points

This code demonstrates:
- ESP32 WiFi connectivity
- NTP time synchronization
- WebSocket real-time communication
- MPU6050 sensor interfacing
- Chart.js data visualization
- Timezone handling
- CSV data export
- Web dashboard creation

## 🔄 Next Steps

After testing with Dhaka timezone:

1. **Analyze Results**
   - Review downloaded CSV data with timestamps
   - Identify optimal thresholds
   - Document crash patterns with exact times

2. **Transfer Settings**
   - Use tested thresholds in main system
   - Update `full-code.ino` or `smart-helmet-web-dashboard.ino`
   - Apply timezone support if needed

3. **Deploy Full System**
   - Add GPS, SIM800L, other components
   - Use validated crash detection logic
   - Consider adding timezone support to main system
   - Confident in crash detection accuracy!

---

**Happy Testing with Real Dhaka Time! 🧪🛡️🇧🇩**
