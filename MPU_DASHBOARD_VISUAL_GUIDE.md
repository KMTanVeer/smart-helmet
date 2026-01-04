# MPU Test Dashboard - Visual Guide

## Dashboard Interface Overview

```
┌─────────────────────────────────────────────────────────────┐
│                   🧪 MPU Test Dashboard                      │
│          Real-Time Crash Detection Testing & Data            │
│                        Recording                             │
├─────────────────────────────────────────────────────────────┤
│  Status: Monitoring | Crashes: 3 | Data Points: 248         │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   📊 Real-Time Values                        │
├──────────────────┬─────────────────┬─────────────────────────┤
│  Acceleration    │   Gyroscope     │   Acc Threshold         │
│     2.87 g       │    134.5 °/s    │       2.50              │
├──────────────────┴─────────────────┴─────────────────────────┤
│                    Gyro Threshold                            │
│                       120.0                                  │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│         ⚙️ Crash Detection Configuration                     │
├─────────────────────────────────────────────────────────────┤
│  [2.5] Acc Threshold  [120] Gyro Threshold  [30] Impact ms  │
│  [Update Thresholds]  [Get Current Settings]                │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│              📈 Live Acceleration Graph                      │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  3.0┤                        🔴                             │
│  2.5┤         ╱‾‾╲      🔴    ╲                             │
│  2.0┤    ╱‾‾╯    ╲    ╱  ╲    ╲                            │
│  1.5┤  ╱          ╲  ╱    ╲    ╲                           │
│  1.0┤‾‾            ╲╱      ‾‾‾‾  ‾‾‾                        │
│  0.5┤                                                        │
│     └────────────────────────────────────────────────────   │
│                    Time (50 points) →                        │
│                                                              │
│  🔴 = Crash detected and marked in RED                      │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│              📈 Live Gyroscope Graph                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│ 200┤                    🔴                                   │
│ 150┤           ╱‾╲   🔴  ╲                                  │
│ 100┤      ╱‾‾╯   ╲ ╱  ╲  ╲                                 │
│  50┤  ╱‾‾╯         V    ╲  ╲                                │
│   0┤‾‾                   ‾‾  ‾‾‾                            │
│     └────────────────────────────────────────────────────   │
│                    Time (50 points) →                        │
│                                                              │
│  🔴 = Crash detected and marked in RED                      │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                   🚨 Crash History                           │
├─────────────────────────────────────────────────────────────┤
│  [Refresh History]  [Download CSV]  [Clear All Data]        │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ 🚨 CRASH DETECTED                                      │  │
│  │ Time: 2026-01-04 14:23:45                             │  │
│  │ Acceleration: 3.87 g                                   │  │
│  │ Gyroscope: 145.67 °/s                                  │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ 🚨 CRASH DETECTED                                      │  │
│  │ Time: 2026-01-04 14:15:23                             │  │
│  │ Acceleration: 2.98 g                                   │  │
│  │ Gyroscope: 132.45 °/s                                  │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                              │
│  ┌───────────────────────────────────────────────────────┐  │
│  │ 🚨 CRASH DETECTED                                      │  │
│  │ Time: 2026-01-04 14:05:12                             │  │
│  │ Acceleration: 3.21 g                                   │  │
│  │ Gyroscope: 156.78 °/s                                  │  │
│  └───────────────────────────────────────────────────────┘  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

## Connection Status Indicator

Located at top-right corner:

**Connected:**
```
┌──────────────────┐
│ ● CONNECTED      │  ← Green background
└──────────────────┘
```

**Disconnected:**
```
┌──────────────────┐
│ ● DISCONNECTED   │  ← Red background
└──────────────────┘
```

## Graph Legend

### Acceleration Graph (Blue)
- **Normal line**: Blue color
- **Crash points**: 🔴 Large red dots
- **Y-axis**: Acceleration in g-force (0-5g typically)
- **X-axis**: Time (last 50 readings)

### Gyroscope Graph (Green)
- **Normal line**: Green color
- **Crash points**: 🔴 Large red dots
- **Y-axis**: Rotation in degrees/second (0-300 typically)
- **X-axis**: Time (last 50 readings)

## What Different States Look Like

### Normal Operation
```
Status: Monitoring (green)
Acceleration: ~1.0g (gravity)
Gyroscope: <10 °/s (still)
Graphs: Flat, stable lines
```

### Vibration/Movement (No Crash)
```
Status: Monitoring (green)
Acceleration: 1.5-2.0g
Gyroscope: 20-50 °/s
Graphs: Small fluctuations, no red dots
```

### Crash Detected!
```
Status: CRASH DETECTED! (red) ← Flashes for 3 seconds
Acceleration: >2.5g
Gyroscope: >120 °/s
Graphs: Large spike with 🔴 red marker
Crash History: New entry added at top
```

## Mobile View

On mobile phones, the layout stacks vertically:

```
┌────────────────────┐
│  🧪 MPU Test       │
│  Dashboard         │
├────────────────────┤
│  Status: OK        │
│  Crashes: 3        │
├────────────────────┤
│  📊 Values         │
│  Acc: 1.02 g       │
│  Gyro: 3.4 °/s     │
├────────────────────┤
│  ⚙️ Configuration  │
│  [2.5]             │
│  [120]             │
│  [30]              │
│  [Update]          │
├────────────────────┤
│  📈 Acc Graph      │
│  [Graph area]      │
├────────────────────┤
│  📈 Gyro Graph     │
│  [Graph area]      │
├────────────────────┤
│  🚨 Crash History  │
│  [Crash 1]         │
│  [Crash 2]         │
│  [Crash 3]         │
└────────────────────┘
```

## Color Scheme

- **Primary**: Purple gradient background (#667eea to #764ba2)
- **Cards**: White with shadow
- **Success/Normal**: Green (#22c55e)
- **Warning**: Orange (#f59e0b)
- **Error/Crash**: Red (#ef4444)
- **Info**: Blue (#3b82f6)
- **Acceleration line**: Blue (#3b82f6)
- **Gyroscope line**: Green (#10b981)
- **Crash markers**: RED (#ef4444) - Large dots

## Button Actions

### Update Thresholds
```
Before: Current settings applied
Click → Sends new values to ESP32
After: Dashboard updates with new thresholds
```

### Get Current Settings
```
Click → Fetches values from ESP32
Updates: All threshold input fields
Displays: Current values in real-time section
```

### Download CSV
```
Click → Generates CSV file
Browser: Downloads file instantly
Filename: Contains all recorded data
```

### Clear All Data
```
Click → Shows confirmation dialog
Confirm → Deletes all stored data
Result: Crash history cleared, counter reset
```

### Refresh History
```
Click → Fetches crash list from ESP32
Updates: Crash history section
Shows: All crashes with details
```

## Real-Time Updates

**Update Rate:** 5 Hz (5 times per second)

**What updates in real-time:**
- ✅ Current acceleration value
- ✅ Current gyroscope value
- ✅ Both graphs (new data points)
- ✅ Crash markers (when detected)
- ✅ System status
- ✅ Crash count

**What needs manual refresh:**
- ⚙️ Threshold settings (click "Get Current Settings")
- 📋 Crash history list (click "Refresh History")

## CSV Download Format

When you click "Download CSV", you get a file like this:

```
Timestamp (ms),Acceleration (g),Gyroscope (deg/s),Crash Detected
1234,1.02,3.45,NO
1434,0.98,2.31,NO
1634,3.87,145.67,YES
1834,1.05,4.12,NO
2034,1.03,3.78,NO
...
```

**Open with:**
- Microsoft Excel
- Google Sheets
- LibreOffice Calc
- Any spreadsheet software

**Analyze:**
- Sort by crash detected
- Create pivot tables
- Generate your own graphs
- Statistical analysis

## Tips for Best Experience

1. **Use Chrome or Edge** - Best WebSocket support
2. **Keep browser tab active** - Background tabs may pause updates
3. **Good WiFi signal** - Reduces connection drops
4. **Full screen on mobile** - Better graph visibility
5. **Landscape mode** - More graph detail on phone
6. **Download data regularly** - Before clearing or power off

## Troubleshooting Visual Cues

### Connection Issues
```
● DISCONNECTED (red) at top-right
→ Check ESP32 power
→ Check WiFi connection
→ Wait 3 seconds for auto-reconnect
```

### No Graph Movement
```
Flat lines, no updates
→ Check WebSocket connection
→ Refresh browser
→ Check Serial Monitor
```

### Graphs Moving But No Crashes
```
Lines updating, but no red dots
→ Thresholds may be too high
→ Try harder impact
→ Check real-time values
```

### Too Many Red Dots
```
Every movement causes red dots
→ Thresholds too low
→ Increase values
→ Check for vibrations
```

---

**This dashboard makes testing crash detection easy, visual, and effective!**

**Access from anywhere on your WiFi network - phone, tablet, laptop, desktop!**
