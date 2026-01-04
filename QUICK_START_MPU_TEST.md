# Quick Start Guide - MPU Testing Tool

## What is This?

A **simple, standalone testing tool** to validate MPU6050 crash detection before deploying the full smart helmet system.

## Why Use This?

✅ **Test crash detection logic** without building the full system  
✅ **No GPS, No SIM card, No buzzer** - just MPU sensor  
✅ **See real-time data** on your phone/computer  
✅ **Download data for analysis** - CSV format  
✅ **Adjust thresholds** and see results immediately  

## What You Need

### Hardware (Only 2 items!)
1. ESP32 board
2. MPU6050 sensor

### Software (3 libraries)
1. **WebSockets** by Markus Sattler
2. **ArduinoJson** by Benoit Blanchon (v6.x)
3. **Wire** (built-in)

## 5-Minute Setup

### Step 1: Install Libraries
```
Arduino IDE → Sketch → Include Library → Manage Libraries
Search and install: "WebSockets" and "ArduinoJson"
```

### Step 2: Wire the MPU6050
```
MPU6050 VCC  → ESP32 3.3V
MPU6050 GND  → ESP32 GND
MPU6050 SDA  → ESP32 GPIO 21
MPU6050 SCL  → ESP32 GPIO 22
```

### Step 3: Configure WiFi
```cpp
const char* WIFI_SSID = "Your_WiFi_Name";
const char* WIFI_PASSWORD = "Your_WiFi_Password";
```

### Step 4: Upload Code
```
File: mpu-test-web-dashboard.ino
Board: ESP32 Dev Module
Upload Speed: 115200
```

### Step 5: Open Dashboard
```
1. Open Serial Monitor (115200 baud)
2. Find IP address (e.g., 192.168.1.100)
3. Open browser on phone/computer
4. Go to: http://192.168.1.100
```

## What You'll See

### Dashboard Features

📊 **Live Graphs**
- Blue line = Acceleration
- Green line = Gyroscope
- 🔴 Red dots = Crash detected!

⚙️ **Configuration**
- Change thresholds on the fly
- See current settings
- Update instantly

🚨 **Crash History**
- All detected crashes listed
- Timestamp, acceleration, gyroscope
- Red background for each crash

💾 **Data Download**
- Click "Download CSV"
- Opens in Excel/Google Sheets
- Analyze patterns offline

## Testing Crash Detection

### Test 1: Baseline (No Movement)
**Expected:**
- Acceleration: ~1.0g (gravity)
- Gyroscope: <5 °/s
- No crashes detected

### Test 2: Gentle Shake
**Expected:**
- Acceleration: 1.5-2.0g
- Gyroscope: 20-50 °/s
- Should NOT trigger crash (if thresholds are 2.5g / 120°/s)

### Test 3: Hard Impact
**Expected:**
- Acceleration: >2.5g
- Gyroscope: >120 °/s
- 🔴 Crash detected! Red marker appears

### Test 4: Drop Test (on soft surface)
**Expected:**
- Brief spike in both values
- Should trigger crash
- Marked in red on graphs

## Adjusting Sensitivity

### Too Sensitive (False Alarms)
```
Increase thresholds:
Acceleration: 2.5 → 3.0g
Gyroscope: 120 → 150 °/s
```

### Not Sensitive Enough (Missing Crashes)
```
Decrease thresholds:
Acceleration: 2.5 → 2.0g
Gyroscope: 120 → 100 °/s
```

### For Motorcycle Testing
```
Recommended:
Acceleration: 2.0 - 2.5g
Gyroscope: 100 - 120 °/s
Impact Time: 30ms
```

## CSV Data Format

Download gives you:
```csv
Timestamp (ms),Acceleration (g),Gyroscope (deg/s),Crash Detected
1234,1.02,3.45,NO
1434,0.98,2.31,NO
1634,3.87,145.67,YES  ← Crash!
1834,1.05,4.12,NO
```

## Common Issues

### "WiFi connection failed"
- Check SSID and password
- Use 2.4GHz WiFi (not 5GHz)
- Move closer to router

### "WebSocket DISCONNECTED"
- Normal! It auto-reconnects in 3 seconds
- Refresh browser if needed

### "Erratic readings"
- Check MPU6050 wiring
- Keep still during calibration (first 2 seconds)
- Ensure 3.3V power

### "No crashes detected"
- Lower thresholds
- Check Serial Monitor for values
- Try harder impact

### "Too many false crashes"
- Increase thresholds
- Check for vibrations
- Secure MPU6050 firmly

## Next Steps

Once you've validated crash detection:

1. **Record Test Data**
   - Multiple scenarios (riding, parking, dropping)
   - Download CSV for each
   - Identify optimal thresholds

2. **Analyze Results**
   - What values trigger real crashes?
   - What values are false positives?
   - Document your findings

3. **Use in Main System**
   - Transfer tested thresholds to `full-code.ino` or `smart-helmet-web-dashboard.ino`
   - Update `ACC_THRESHOLD` and `GYRO_THRESHOLD`
   - Deploy with confidence!

## Files

- **mpu-test-web-dashboard.ino** - Main code
- **MPU_TEST_README.md** - Full documentation
- **QUICK_START_MPU_TEST.md** - This guide

## Support

Questions? Check:
1. Serial Monitor output
2. MPU_TEST_README.md (detailed guide)
3. Main README.md (general info)

---

**Happy Testing! 🧪🛡️**

**Remember:** This is a TESTING TOOL ONLY. No emergency features. Use to validate logic before full deployment.
