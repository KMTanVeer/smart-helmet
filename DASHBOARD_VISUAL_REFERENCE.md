# Smart Helmet Dashboard Visual Reference

## Dashboard Layout

The dashboard is divided into several sections, each serving a specific purpose for real-time monitoring and configuration.

---

## 🎯 Main Dashboard Screen

```
┌─────────────────────────────────────────────────────────────┐
│                    ● CONNECTED (Green/Red)                  │ ← Connection Status
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│         🛡️ Smart Helmet Dashboard                          │
│    Real-Time Crash Detection with Accelerometer &          │
│              Gyroscope Monitoring                           │
│                                                             │
│  Status: Monitoring  |  Crashes: 0  |  Data Points: 50    │
└─────────────────────────────────────────────────────────────┘
```

---

## 📊 Real-Time Values Section

```
┌─────────────────────────────────────────────────────────────┐
│                    📊 Real-Time Values                      │
├──────────────┬──────────────┬──────────────┬───────────────┤
│ Acceleration │  Gyroscope   │     Acc      │     Gyro      │
│     (g)      │    (°/s)     │  Threshold   │  Threshold    │
│              │              │              │               │
│    1.05      │    15.3      │    2.50      │    120.0      │
└──────────────┴──────────────┴──────────────┴───────────────┘
```

These boxes update in real-time showing:
- Current acceleration magnitude
- Current gyroscope magnitude  
- Configured thresholds

---

## ⚙️ MPU Threshold Configuration

```
┌─────────────────────────────────────────────────────────────┐
│              ⚙️ MPU Threshold Configuration                 │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌────────────┐  ┌────────────┐  ┌────────────┐           │
│  │ Acc: [2.5] │  │ Gyro:[120] │  │ Time: [30] │           │
│  └────────────┘  └────────────┘  └────────────┘           │
│                                                             │
│  ┌──────────────────┐  ┌──────────────────┐               │
│  │ Update Thresholds│  │ Get Current Set. │               │
│  └──────────────────┘  └──────────────────┘               │
└─────────────────────────────────────────────────────────────┘
```

Input fields allow you to:
- Change acceleration threshold (1-10 g)
- Change gyroscope threshold (50-300 °/s)
- Change impact duration (10-100 ms)

---

## 📈 Live Acceleration Graph

```
┌─────────────────────────────────────────────────────────────┐
│                📈 Live Acceleration Graph                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  4.0g │                    🔴 ← CRASH POINT                │
│       │                   /  \                              │
│  3.0g │                  /    \                             │
│       │                 /      \                            │
│  2.0g │    ___________/        \___________                │
│       │   /                                 \               │
│  1.0g │__/                                   \__            │
│       │                                                     │
│  0.0g └────────────────────────────────────────────→        │
│                        Time                                 │
└─────────────────────────────────────────────────────────────┘
```

**Features:**
- Blue line for normal readings
- **RED dots (●)** mark crash detection points
- Smooth animation
- Hover tooltips show "🚨 CRASH DETECTED"
- Auto-scaling Y-axis

---

## 📈 Live Gyroscope Graph

```
┌─────────────────────────────────────────────────────────────┐
│                📈 Live Gyroscope Graph                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ 200°/s│                   🔴 ← CRASH POINT                 │
│       │                  / \                                │
│ 150°/s│                 /   \                               │
│       │                /     \                              │
│ 100°/s│    ___________/       \_________                   │
│       │   /                             \                   │
│  50°/s│__/                               \__                │
│       │                                                     │
│   0°/s└────────────────────────────────────────────→        │
│                        Time                                 │
└─────────────────────────────────────────────────────────────┘
```

**Features:**
- Green line for normal readings
- **RED dots (●)** mark crash detection points
- Synchronized with acceleration graph
- Real-time updates every 200ms

---

## 🚨 Crash History Section

```
┌─────────────────────────────────────────────────────────────┐
│                    🚨 Crash History                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌─────────────────┐          │
│  │ Refresh  │  │Download  │  │Download Crash   │          │
│  │ History  │  │   CSV    │  │      CSV        │          │
│  └──────────┘  └──────────┘  └─────────────────┘          │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 🚨 CRASH DETECTED                                   │   │
│  │ Date/Time (BD): 2024-01-15 14:30:45                │   │
│  │ Acceleration: 3.20 g                                │   │
│  │ Gyroscope: 145.70 °/s                               │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │ 🚨 CRASH DETECTED                                   │   │
│  │ Date/Time (BD): 2024-01-15 13:15:22                │   │
│  │ Acceleration: 2.85 g                                │   │
│  │ Gyroscope: 132.40 °/s                               │   │
│  └─────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

**Features:**
- Red background for crash items
- Timestamp in BD Dhaka timezone (UTC+6)
- Scrollable list (max 400px height)
- Download options for data export

---

## 📥 CSV Download Options

### Option 1: Download CSV (Real-Time Data)

Downloads the current session's data buffer (up to 1000 points):

```
Timestamp (ms),Date/Time (BD),Acceleration (g),Gyroscope (deg/s),Crash Detected
15234,2024-01-15 14:30:45,1.05,15.3,NO
15434,2024-01-15 14:30:45,1.08,16.2,NO
15634,2024-01-15 14:30:46,3.20,145.7,YES
15834,2024-01-15 14:30:46,2.95,138.2,NO
```

### Option 2: Download Crash CSV (SPIFFS Data)

Downloads all crashes logged to SPIFFS with GPS data:

```
Timestamp,DateTime,Latitude,Longitude,AccMag,GyroMag,Battery%
15634,2024-01-15 14:30:46,23.810331,90.412521,3.20,145.7,85
28451,2024-01-15 15:45:12,23.811245,90.413678,2.85,132.4,82
```

---

## 🎨 Color Scheme

### Status Colors

- **Green (#22c55e)** - Normal operation, connected
- **Yellow (#f59e0b)** - Warning/monitoring
- **Red (#ef4444)** - Crash detected, error, disconnected
- **Blue (#667eea)** - Primary theme color
- **Purple (#764ba2)** - Secondary theme color

### Graph Colors

- **Acceleration Line**: Blue (#3b82f6)
- **Gyroscope Line**: Green (#10b981)
- **Crash Points**: Red (#ef4444)
- **Normal Points**: Same as line color

---

## 📱 Mobile View

On mobile devices, the dashboard automatically adapts:

```
┌─────────────────────┐
│   ● CONNECTED       │
│                     │
│  Smart Helmet       │
│    Dashboard        │
│                     │
│ Status: Monitoring  │
│ Crashes: 0          │
│ Data Points: 50     │
│                     │
│ ┌─────────────────┐ │
│ │ Acceleration    │ │
│ │     1.05 g      │ │
│ └─────────────────┘ │
│                     │
│ ┌─────────────────┐ │
│ │  Gyroscope      │ │
│ │    15.3 °/s     │ │
│ └─────────────────┘ │
│                     │
│ [Acc Threshold]     │
│ [Gyro Threshold]    │
│ [Impact Time]       │
│ [Update Thresholds] │
│                     │
│ [Graph Area]        │
│                     │
│ [Crash History]     │
└─────────────────────┘
```

Controls stack vertically for easy thumb navigation.

---

## 🔄 Real-Time Updates

The dashboard updates via WebSocket every **200ms** (5 Hz):

1. **Sensor readings** update continuously
2. **Graphs** scroll left as new data arrives
3. **Crash markers** appear immediately when detected
4. **Status** changes from "Monitoring" to "CRASH DETECTED!" for 3 seconds
5. **Connection status** shows green when connected

---

## 🎯 Visual Indicators

### Connection Status (Top-Right)

```
● CONNECTED    (Green background, white text)
● DISCONNECTED (Red background, white text)
```

### System Status

```
Status: Monitoring           (Green text)
Status: CRASH DETECTED!      (Red text, 3-second duration)
```

### Crash Markers on Graph

- Normal points: Small dots (3px)
- Crash points: Large red dots (10px)
- Hover: Tooltip appears with crash note

---

## 💡 User Interaction

### Buttons

All buttons have hover effects:
- **Hover**: Slightly elevated with shadow
- **Click**: Momentary press effect
- **Color**: Primary purple theme

### Input Fields

- **Border**: Purple outline
- **Focus**: Highlighted border
- **Validation**: Min/max values enforced

### Graphs

- **Zoom**: Not available (continuous scroll)
- **Hover**: Show data point values
- **Legend**: Always visible
- **Animation**: Smooth, no delay

---

## 🌐 Browser Compatibility

✅ Chrome (recommended)  
✅ Firefox  
✅ Safari (iOS/macOS)  
✅ Edge  
✅ Mobile browsers  

⚠️ Internet Explorer not supported

---

## 📊 Dashboard Performance

- **Update Rate**: 5 Hz (every 200ms)
- **Graph Points**: 100 visible points (configurable)
- **Data Buffer**: 1000 points max
- **Memory Usage**: ~50KB per 1000 points
- **Latency**: < 100ms typical

---

## 🎬 Animation Effects

1. **Button Hover**: Lift with shadow
2. **Button Click**: Press down
3. **Graph Update**: Smooth scroll
4. **Crash Detection**: Instant red marker
5. **Status Change**: 3-second highlight

---

## 📐 Responsive Breakpoints

- **Desktop**: > 768px (multi-column layout)
- **Tablet**: 768px (2-column layout)
- **Mobile**: < 768px (single column)

All elements scale appropriately for screen size.

---

**End of Visual Reference**

For technical setup, see DASHBOARD_SETUP_GUIDE.md
