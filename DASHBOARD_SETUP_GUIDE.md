# Smart Helmet Real-Time Crash Detection Dashboard Setup Guide

## Overview

The enhanced `final-code.ino` now includes a **real-time web dashboard** for monitoring crash detection with live accelerometer and gyroscope data visualization.

## Features

✅ **Real-Time Data Streaming** - Live graphs showing accelerometer and gyroscope readings  
✅ **Crash Detection Visualization** - Crashes highlighted in RED with "Crash Detected" notes  
✅ **Configurable Thresholds** - Change MPU6050 thresholds via web interface  
✅ **CSV Download** - Download both real-time data and SPIFFS crash logs  
✅ **BD Dhaka Timezone** - All timestamps in Bangladesh Dhaka time (UTC+6)  
✅ **Mobile Responsive** - Works on phones, tablets, and computers  

## Hardware Requirements

- ESP32 Development Board
- MPU6050 Accelerometer/Gyroscope
- GPS Module (Ublox Neo-8M)
- SIM800L GSM Module
- OLED Display (SSD1306)
- Passive Buzzer
- Push Button
- Battery with voltage sensor

## Software Requirements

### Arduino Libraries

Install the following libraries via Arduino IDE Library Manager:

1. **WiFi** (built-in with ESP32)
2. **WebServer** (built-in with ESP32)
3. **WebSocketsServer** by Markus Sattler
4. **ArduinoJson** by Benoit Blanchon
5. **Wire** (built-in)
6. **TinyGPSPlus** by Mikal Hart
7. **Adafruit_GFX** by Adafruit
8. **Adafruit_SSD1306** by Adafruit
9. **SPIFFS** (built-in with ESP32)

## Configuration Steps

### 1. WiFi Configuration

Open `final-code.ino` and modify the WiFi credentials:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ CHANGE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ CHANGE THIS
```

### 2. Upload to ESP32

1. Connect ESP32 to your computer via USB
2. Select board: **ESP32 Dev Module**
3. Select correct COM port
4. Click **Upload**

### 3. Find ESP32 IP Address

1. Open Serial Monitor (115200 baud)
2. Wait for ESP32 to connect to WiFi
3. Look for the IP address in the output:

```
✅ WiFi connected!
📱 IP Address: 192.168.1.100
🌐 Open browser to: http://192.168.1.100
```

### 4. Access Dashboard

Open a web browser on any device connected to the same WiFi network and navigate to:

```
http://[ESP32-IP-ADDRESS]
```

Example: `http://192.168.1.100`

## Dashboard Features

### Real-Time Monitoring

The dashboard displays:

- **Current Acceleration** - Real-time g-force reading
- **Current Gyroscope** - Real-time angular velocity in °/s
- **Acceleration Threshold** - Current configured threshold
- **Gyroscope Threshold** - Current configured threshold

### Live Graphs

Two real-time graphs show:

1. **Acceleration Graph** - Magnitude of acceleration over time
2. **Gyroscope Graph** - Magnitude of rotation over time

**Crash Detection:** When a crash is detected, the data point is marked in **RED** with a larger point size. Hovering over the point shows "🚨 CRASH DETECTED" in the tooltip.

### Threshold Configuration

Change the crash detection sensitivity:

- **Acc Threshold (g)** - Minimum acceleration for crash (1-10 g)
- **Gyro Threshold (°/s)** - Minimum rotation for crash (50-300 °/s)
- **Impact Time (ms)** - Duration threshold must be exceeded (10-100 ms)

Click **"Update Thresholds"** to apply changes immediately.

### Crash History

View all detected crashes with:

- Date/Time in BD Dhaka timezone
- Acceleration magnitude at crash
- Gyroscope magnitude at crash

### Data Download

Two CSV download options:

1. **Download CSV** - Real-time data from current session (up to 1000 points)
   - Includes: Timestamp, Date/Time, Acceleration, Gyroscope, Crash flag

2. **Download Crash CSV** - All crashes logged to SPIFFS
   - Includes: Timestamp, Date/Time, GPS coordinates, Acceleration, Gyroscope, Battery%

## Timezone Configuration

All timestamps are shown in **Bangladesh Dhaka timezone (UTC+6)**.

The system automatically synchronizes with NTP servers on startup. Date/Time format:

```
YYYY-MM-DD HH:MM:SS
Example: 2024-01-15 14:30:45
```

## Troubleshooting

### Dashboard Won't Load

1. **Check WiFi Connection**
   - Ensure ESP32 connected to WiFi (check Serial Monitor)
   - Verify you're on the same network

2. **Check IP Address**
   - IP address shown in Serial Monitor
   - Try accessing from different device

3. **Firewall Issues**
   - Some routers may block WebSocket on port 81
   - Try accessing from mobile phone

### No Real-Time Data

1. **WebSocket Connection**
   - Look for connection status indicator (top-right)
   - Should show "● CONNECTED" in green

2. **Refresh Page**
   - Try refreshing the browser
   - Clear browser cache

3. **Check Serial Monitor**
   - Verify MPU6050 is reading data
   - Check for error messages

### Wrong Timezone

The system uses NTP to sync time. If time is incorrect:

1. Check internet connection
2. Wait a few minutes for NTP sync
3. Restart ESP32

## Technical Details

### WebSocket Communication

- Server Port: **81**
- Update Rate: Every **200ms** (5 Hz)
- Data Format: JSON

Example sensor data message:
```json
{
  "type": "sensor",
  "acc": 1.05,
  "gyro": 15.3,
  "crash": false
}
```

Example crash event message:
```json
{
  "type": "crash",
  "count": 3,
  "acc": 3.2,
  "gyro": 145.7,
  "dateTime": "2024-01-15 14:30:45"
}
```

### Data Storage

- **RAM Buffer**: 1000 data points (circular buffer)
- **SPIFFS**: Crash logs saved permanently
- **CSV Format**: Compatible with Excel and data analysis tools

### Performance

- Dashboard works on:
  - Desktop browsers (Chrome, Firefox, Edge, Safari)
  - Mobile browsers (iOS Safari, Android Chrome)
  - Tablets
  
- Recommended browsers:
  - Chrome (best performance)
  - Firefox
  - Safari (iOS/macOS)

## Advanced Configuration

### Change Data Update Rate

Modify WebSocket send interval in `loop()`:

```cpp
if (millis() - lastWebSocketSend >= 200) {  // Change 200 to desired ms
```

### Increase Data History

Change buffer size (uses more RAM):

```cpp
#define MAX_DATA_POINTS 1000  // Increase for more history
```

### Change Graph Display Points

Modify in dashboard HTML:

```javascript
const maxDataPoints = 100;  // Increase to show more points
```

## Safety Notes

⚠️ **Important:**

1. Do NOT change thresholds while riding
2. Test new thresholds in a safe environment
3. Default thresholds (2.5g, 120°/s) are recommended
4. Keep emergency contacts updated in the code

## Support

For issues or questions:

1. Check Serial Monitor for error messages
2. Verify all libraries are installed
3. Ensure ESP32 has adequate power supply
4. Test with a stable WiFi connection

## Version Information

- **Version**: 2.1 (Web Dashboard)
- **Date**: January 2024
- **Author**: Smart Helmet Enhanced Project
- **Board**: ESP32 Dev Module
- **Core**: ESP32 Arduino Core

---

**Happy Monitoring! 🛡️**
