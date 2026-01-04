# Crash Data Logging & Graph Generation - Complete Guide

## Overview

The enhanced version (`smart-helmet-enhanced.ino`) includes a complete crash data logging system with graph generation capabilities. This guide explains how to use it.

## How It Works

### 1. Data Collection (Automatic)

When a crash is detected, the system automatically logs:
- **Timestamp**: Milliseconds since system boot
- **GPS Coordinates**: Latitude and longitude
- **Acceleration**: Magnitude in g-force
- **Gyroscope**: Magnitude in degrees per second
- **Battery Level**: Percentage at time of crash

### 2. Data Storage (SPIFFS)

Data is stored in ESP32's built-in flash memory:
- **File**: `/crashes.csv`
- **Format**: CSV (Comma-Separated Values)
- **Capacity**: ~30,000 crash records (1.5MB SPIFFS)
- **Persistent**: Data survives power cycles

### 3. Data Retrieval

Multiple methods to get your crash data:

#### Method A: Serial Monitor (Quick Check)
```cpp
// Add to loop() temporarily:
if (Serial.available() && Serial.read() == 'D') {
  displayCrashLogs();  // Already implemented in code
}
```
Then press 'D' in Serial Monitor to see all crashes.

#### Method B: Web Server (Recommended for Production)

Add this code to `smart-helmet-enhanced.ino`:

```cpp
#include <WebServer.h>

WebServer server(80);

void handleCrashData() {
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "No crash data found");
    return;
  }
  
  server.streamFile(file, "text/csv");
  file.close();
}

void setup() {
  // ... existing setup code ...
  
  // Add WiFi and web server
  WiFi.begin("YOUR_WIFI_SSID", "YOUR_PASSWORD");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  
  server.on("/crashes.csv", handleCrashData);
  server.begin();
  Serial.print("Web server at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  // ... existing loop code ...
  server.handleClient();
}
```

Then access: `http://[ESP32-IP]/crashes.csv`

#### Method C: Bluetooth File Transfer (Mobile App)

Add Bluetooth Serial functionality:

```cpp
#include <BluetoothSerial.h>

BluetoothSerial BT;

void setup() {
  BT.begin("SmartHelmet");
  // When connected, send file via BT
}
```

#### Method D: SD Card (Maximum Storage)

If you add SD card module:
- Store on SD instead of SPIFFS
- Remove SD card and read on PC
- Unlimited storage capacity

### 4. Graph Generation

#### Option 1: Python Script (Included)

**What's Included:**
- File: `visualize_crashes.py`
- Generates 5 types of outputs:
  1. Acceleration graph (PNG)
  2. Gyroscope graph (PNG)
  3. Battery level graph (PNG)
  4. Combined dashboard (PNG)
  5. Interactive map (HTML)

**Usage:**
```bash
# Install requirements
pip install pandas matplotlib folium

# Download crashes.csv from ESP32
# Place in same folder as script

# Run visualization
python visualize_crashes.py
```

**Output Files:**
```
crash_acceleration.png  - Acceleration over crashes
crash_gyro.png         - Rotation over crashes
crash_battery.png      - Battery levels
crash_combined.png     - 4-panel dashboard
crash_map.html         - Interactive map
```

#### Option 2: Excel/Google Sheets (Simple)

1. Download `crashes.csv`
2. Open in Excel or Google Sheets
3. Select data columns
4. Insert → Chart
5. Choose chart type:
   - Line chart for trends
   - Scatter for GPS locations
   - Bar chart for comparisons

**Example Excel Charts:**
- X-axis: Crash Event Number
- Y-axis: Acceleration or Gyroscope
- Add threshold line at 2.5g

#### Option 3: MATLAB (Scientific)

```matlab
% Load CSV
data = readtable('crashes.csv');

% Plot acceleration
figure;
plot(data.Timestamp, data.AccMag, '-o');
xlabel('Time (ms)');
ylabel('Acceleration (g)');
title('Crash Detection - Acceleration');
grid on;

% Plot GPS locations
figure;
geoplot(data.Latitude, data.Longitude, 'r*', 'MarkerSize', 10);
title('Crash Locations');
```

#### Option 4: Web Dashboard (Advanced)

Create real-time dashboard with Chart.js:

```html
<!DOCTYPE html>
<html>
<head>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
  <canvas id="crashChart"></canvas>
  
  <script>
    // Fetch crash data
    fetch('http://[ESP32-IP]/crashes.csv')
      .then(response => response.text())
      .then(csv => {
        // Parse CSV and create chart
        const lines = csv.split('\n');
        const data = lines.slice(1).map(line => {
          const [timestamp, lat, lon, acc, gyro, bat] = line.split(',');
          return { timestamp, acc: parseFloat(acc), gyro: parseFloat(gyro) };
        });
        
        new Chart(document.getElementById('crashChart'), {
          type: 'line',
          data: {
            labels: data.map(d => d.timestamp),
            datasets: [{
              label: 'Acceleration (g)',
              data: data.map(d => d.acc),
              borderColor: 'red'
            }]
          }
        });
      });
  </script>
</body>
</html>
```

#### Option 5: Mobile App (Future Enhancement)

Create Flutter/React Native app:
- Connect via Bluetooth
- Download crash data
- Display graphs on phone
- Export to cloud storage

## Example Graphs

### 1. Acceleration Over Time
```
Acceleration (g)
    4.0 |                    *
    3.5 |          *    *
    3.0 |     *         
    2.5 |---------------------------  (threshold)
    2.0 | *
    1.5 |
        +---------------------------
         1   2   3   4   5   6   7
                Crash Event
```

### 2. GPS Location Map
```
Map showing:
- Red markers for each crash
- Lines connecting sequential crashes
- Popup with crash details
- Google Maps integration
```

### 3. Battery Level
```
Battery (%)
   100 |████
    80 |    ████
    60 |        ████
    40 |            ░░░░
    20 |                ░░░░
     0 +---------------------------
        1   2   3   4   5   6   7
              Crash Event
        
Green = >50%, Orange = 20-50%, Red = <20%
```

### 4. Combined Dashboard
```
┌─────────────────┬─────────────────┐
│ Acceleration    │ Gyroscope       │
│ [Line Graph]    │ [Line Graph]    │
├─────────────────┼─────────────────┤
│ Battery Levels  │ Statistics      │
│ [Bar Chart]     │ [Text Summary]  │
└─────────────────┴─────────────────┘
```

## Data Analysis Examples

### 1. Find Most Severe Crash
```python
import pandas as pd
df = pd.read_csv('crashes.csv')
severe = df.loc[df['AccMag'].idxmax()]
print(f"Most severe crash: {severe['AccMag']}g at {severe['Latitude']}, {severe['Longitude']}")
```

### 2. Analyze Crash Patterns
```python
# Crashes by location
location_counts = df.groupby(['Latitude', 'Longitude']).size()
print("Crash hotspots:", location_counts.nlargest(5))

# Battery correlation
low_battery_crashes = df[df['Battery%'] < 20]
print(f"Crashes with low battery: {len(low_battery_crashes)}")
```

### 3. Export to JSON
```python
import json
data = df.to_dict(orient='records')
with open('crashes.json', 'w') as f:
    json.dump(data, f, indent=2)
```

## Real-Time Monitoring

### Stream Data to Cloud

Add to `smart-helmet-enhanced.ino`:

```cpp
#include <HTTPClient.h>

void sendToCloud(float lat, float lon, float acc, float gyro, int bat) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    String url = "https://your-api.com/crashes";
    String payload = "{\"lat\":" + String(lat, 6) + 
                     ",\"lon\":" + String(lon, 6) +
                     ",\"acc\":" + String(acc, 2) +
                     ",\"gyro\":" + String(gyro, 2) +
                     ",\"bat\":" + String(bat) + "}";
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.POST(payload);
    http.end();
  }
}

// Call after logging:
logCrashData(lat, lon, accMag, gyroMag, batteryPercent);
sendToCloud(lat, lon, accMag, gyroMag, batteryPercent);
```

## Frequently Asked Questions

**Q: How many crashes can be stored?**
A: About 30,000 records (1.5MB SPIFFS). Each record is ~50 bytes.

**Q: What happens when storage is full?**
A: System will still work, but logging will fail. Implement log rotation or increase partition size.

**Q: Can I export data automatically?**
A: Yes, add WiFi/Bluetooth and send to cloud or mobile app.

**Q: Do I need Python installed on ESP32?**
A: No! Python runs on your PC/laptop. ESP32 only stores CSV.

**Q: Can I view graphs on the OLED display?**
A: Possible but limited. 128x64 is very small. Better to export and view on PC/phone.

**Q: Is crash data deleted on restart?**
A: No, SPIFFS data persists across power cycles.

**Q: How do I clear crash logs?**
A: Add this function and call it:
```cpp
void clearCrashLogs() {
  SPIFFS.remove("/crashes.csv");
  initCrashLogging();
}
```

**Q: Can I add timestamps with date/time?**
A: Yes, get time from GPS or NTP server:
```cpp
String datetime = String(gps.date.year()) + "-" + 
                  String(gps.date.month()) + "-" + 
                  String(gps.date.day());
```

## Summary

**Yes, the system can save crash data with graphs!**

✅ **Data Storage**: CSV format on ESP32 SPIFFS
✅ **Data Retrieval**: Serial, Web, Bluetooth, SD card
✅ **Graph Generation**: Python script included
✅ **Multiple Options**: Excel, MATLAB, web dashboard
✅ **Real-time**: Can stream to cloud
✅ **Persistent**: Data survives power cycles

**Steps to Use:**
1. Upload `smart-helmet-enhanced.ino` to ESP32
2. System automatically logs crashes
3. Download `crashes.csv` (via method of choice)
4. Run `visualize_crashes.py` to generate graphs
5. Analyze data and improve safety!

**Need Help?**
- Check `ENHANCED_VERSION_README.md` for setup
- Run `test-passive-buzzer.ino` to verify hardware
- See `VERSION_COMPARISON.md` for feature comparison

---

📊 **Your crash data is valuable for improving safety!** 📊
