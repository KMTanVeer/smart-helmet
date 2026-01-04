# Web Dashboard Visual Guide

## 🎨 Dashboard Screenshots & Mockups

This guide shows what the Smart Helmet Web Dashboard looks like on different devices.

## 📱 Mobile View

### Home Screen (Portrait)

```
┌─────────────────────────────────┐
│  ● CONNECTED     [Top Right]    │
│                                  │
│  🛡️ Smart Helmet Dashboard      │
│  Real-Time Crash Detection      │
│  & Monitoring System             │
│  ┌──────────────────────────┐   │
│  │ GPS: Connected  🟢       │   │
│  │ Battery: 85%    🔋       │   │
│  │ Crashes: 3      🚨       │   │
│  │ Status: Monitoring ✅    │   │
│  └──────────────────────────┘   │
│                                  │
│  ⚙️ Configuration                │
│  ┌──────────────────────────┐   │
│  │ [Acc: 2.5g            ]  │   │
│  │ [Gyro: 120°/s         ]  │   │
│  │ [Impact: 30ms         ]  │   │
│  │ [Update Thresholds]      │   │
│  │ [Get Current Settings]   │   │
│  └──────────────────────────┘   │
│                                  │
│  📊 Live Acceleration            │
│  ┌──────────────────────────┐   │
│  │     ╱╲    ╱╲    ╱╲       │   │
│  │    ╱  ╲  ╱  ╲  ╱  ╲      │   │
│  │ ──╱────╲╱────╲╱────╲──── │   │
│  │   Blue Line (Normal)      │   │
│  │   🔴 Red Spikes (Crash)  │   │
│  └──────────────────────────┘   │
│                                  │
│  📊 Live Gyroscope               │
│  ┌──────────────────────────┐   │
│  │     ╱╲    ╱╲    ╱╲       │   │
│  │    ╱  ╲  ╱  ╲  ╱  ╲      │   │
│  │ ──╱────╲╱────╲╱────╲──── │   │
│  │   Purple Line (Normal)    │   │
│  │   🔴 Red Spikes (Crash)  │   │
│  └──────────────────────────┘   │
│                                  │
│  🚨 Crash History                │
│  ┌──────────────────────────┐   │
│  │ [Refresh] [Download] [Clear]│
│  │                              │
│  │ ┌────────────────────────┐  │
│  │ │🚨 Crash #1 - 10:30 AM  │  │
│  │ │📍 Lat: 23.810332       │  │
│  │ │📍 Lon: 90.412518       │  │
│  │ │⚡ Acc: 3.45g           │  │
│  │ │🔄 Gyro: 156.7°/s      │  │
│  │ │🔋 Battery: 85%        │  │
│  │ │📝 Note: Pothole on... │  │
│  │ │[Add note...           ]│  │
│  │ │[Save Note] [View Map] │  │
│  │ └────────────────────────┘  │
│  │                              │
│  │ ┌────────────────────────┐  │
│  │ │🚨 Crash #2 - 11:45 AM  │  │
│  │ │... (more crashes)       │  │
│  │ └────────────────────────┘  │
│  └──────────────────────────┘   │
└─────────────────────────────────┘
```

## 💻 Desktop View (Laptop/Tablet)

### Home Screen (Landscape)

```
┌──────────────────────────────────────────────────────────────────┐
│  ● CONNECTED                                        [Top Right]  │
│                                                                   │
│        🛡️ Smart Helmet Dashboard                                │
│        Real-Time Crash Detection & Monitoring System            │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │  GPS: Connected 🟢  │  Battery: 85% 🔋  │  Crashes: 3 🚨  │ │
│  │  Status: Monitoring ✅                                      │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  ⚙️ Configuration                                                │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ [Acc: 2.5g] [Gyro: 120°/s] [Impact: 30ms]                  │ │
│  │ [Update Thresholds] [Get Current Settings]                  │ │
│  └────────────────────────────────────────────────────────────┘ │
│                                                                   │
│  ┌──────────────────────────┬──────────────────────────────┐    │
│  │ 📊 Live Acceleration     │ 📊 Live Gyroscope            │    │
│  ├──────────────────────────┼──────────────────────────────┤    │
│  │      ╱╲    ╱╲    ╱╲     │      ╱╲    ╱╲    ╱╲         │    │
│  │     ╱  ╲  ╱  ╲  ╱  ╲    │     ╱  ╲  ╱  ╲  ╱  ╲        │    │
│  │ ───╱────╲╱────╲╱────╲─  │ ───╱────╲╱────╲╱────╲─      │    │
│  │    Blue Line            │    Purple Line                │    │
│  │    🔴 Red = Crash       │    🔴 Red = Crash            │    │
│  └──────────────────────────┴──────────────────────────────┘    │
│                                                                   │
│  🚨 Crash History                                                │
│  ┌────────────────────────────────────────────────────────────┐ │
│  │ [Refresh History] [Download CSV] [Clear All]                │ │
│  │                                                              │ │
│  │ ┌─────────────────────────────────────────────────────────┐│ │
│  │ │ 🚨 Crash #1 - May 15, 2024 10:30 AM                     ││ │
│  │ │ 📍 Lat: 23.810332  📍 Lon: 90.412518                    ││ │
│  │ │ ⚡ Acc: 3.45g  🔄 Gyro: 156.7°/s  🔋 Battery: 85%      ││ │
│  │ │ 📝 Note: Hit pothole on Main Street near intersection   ││ │
│  │ │ [Edit note...                                          ] ││ │
│  │ │ [Save Note] [View on Map]                               ││ │
│  │ └─────────────────────────────────────────────────────────┘│ │
│  │                                                              │ │
│  │ ┌─────────────────────────────────────────────────────────┐│ │
│  │ │ 🚨 Crash #2 - May 15, 2024 11:45 AM                     ││ │
│  │ │ ... (more crashes)                                       ││ │
│  │ └─────────────────────────────────────────────────────────┘│ │
│  └────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
```

## 🎨 Color Scheme

### Primary Colors
- **Purple Gradient Background**: `#667eea` → `#764ba2`
- **White Cards**: Clean, modern card design
- **Blue Graphs**: `#3b82f6` (Normal acceleration)
- **Purple Graphs**: `#8b5cf6` (Normal gyroscope)
- **Red Alerts**: `#ef4444` (Crash events)

### Status Colors
- **Green** 🟢: Connected, GPS OK, Good Battery (>50%)
- **Orange** 🟠: Warning, Battery Medium (20-50%)
- **Red** 🔴: Disconnected, Crash Alert, Low Battery (<20%)

### UI Elements
- **Buttons**: Purple with white text, hover effects
- **Cards**: White with subtle shadows
- **Inputs**: Clean borders, rounded corners
- **Status Bar**: Light blue background

## 📊 Graph Details

### Normal Operation
```
Acceleration (g)
  3.0 |
  2.5 |-------- Threshold
  2.0 |
  1.5 |      ╱╲        ╱╲
  1.0 |     ╱  ╲      ╱  ╲
  0.5 |────╱────╲────╱────╲────
  0.0 |
      └────────────────────────→ Time
      Blue line, smooth curve
```

### During Crash
```
Acceleration (g)
  4.0 |        🔴
  3.5 |       ╱  ╲
  3.0 |      ╱    ╲
  2.5 |----╱────────╲---- Threshold
  2.0 |   ╱          ╲
  1.5 |  ╱            ╲
  1.0 |─╱──────────────╲────
      └────────────────────────→ Time
      Red spike with large dot marker
```

## 🔔 Crash Alert Visualization

### Crash Card (With Note)
```
┌────────────────────────────────────────┐
│ 🚨 Crash #3 - May 15, 2024 2:15 PM   │ Blue border
│                                        │
│ 📍 Latitude: 23.810332                │
│ 📍 Longitude: 90.412518               │
│                                        │
│ ⚡ Acc: 3.45g    🔄 Gyro: 156.7°/s   │
│ 🔋 Battery: 85%                       │
│                                        │
│ ┌────────────────────────────────────┐│
│ │ 📝 Pothole on Main St near        ││
│ │    the intersection               ││
│ └────────────────────────────────────┘│
│                                        │
│ [Edit note...                        ] │
│ [Save Note] [View on Map]             │
└────────────────────────────────────────┘
```

### Crash Card (Without Note)
```
┌────────────────────────────────────────┐
│ 🚨 Crash #4 - May 15, 2024 3:30 PM   │ Red border
│                                        │
│ 📍 Latitude: 23.811234                │
│ 📍 Longitude: 90.413456               │
│                                        │
│ ⚡ Acc: 4.12g    🔄 Gyro: 178.3°/s   │
│ 🔋 Battery: 82%                       │
│                                        │
│ [Add note...                         ] │
│ [Save Note] [View on Map]             │
└────────────────────────────────────────┘
```

## 🔄 Real-Time Updates

### WebSocket Status Indicator

**Connected**:
```
┌───────────────┐
│ ● CONNECTED   │ Green circle
└───────────────┘
```

**Disconnected**:
```
┌─────────────────┐
│ ● DISCONNECTED  │ Red circle
└─────────────────┘
```

**Reconnecting**:
```
┌──────────────────┐
│ ⟳ RECONNECTING...│ Orange with spinner
└──────────────────┘
```

## 📱 Mobile Features

### Add to Home Screen Icon

When added to mobile home screen, appears as:

```
┌──────┐
│  🛡️  │
│ Smart │
│Helmet │
└──────┘
```

Tap to open like a native app!

### Pull to Refresh

On mobile, pull down from top to refresh:

```
    ↓
┌──────────┐
│ Release  │
│    to    │
│ Refresh  │
└──────────┘
```

### Touch Gestures

- **Tap**: Click buttons, select inputs
- **Scroll**: Browse crash history
- **Pull**: Refresh dashboard
- **Pinch**: Zoom graphs (if enabled)

## 💡 Interactive Elements

### Buttons

**Normal State**:
```
┌───────────────────┐
│ Update Thresholds │ Purple background
└───────────────────┘
```

**Hover State**:
```
┌───────────────────┐
│ Update Thresholds │ Darker purple, lifts up
└───────────────────┘
   ↑ Shadow appears
```

**Pressed State**:
```
┌───────────────────┐
│ Update Thresholds │ Returns to normal position
└───────────────────┘
```

### Input Fields

**Normal**:
```
┌─────────────────────┐
│ 2.5                 │ Purple border
└─────────────────────┘
```

**Focused**:
```
┌═════════════════════┐
║ 2.5█                ║ Thicker border, cursor
└═════════════════════┘
```

## 🎬 Animation Effects

### Page Load
1. Header fades in (0.3s)
2. Status cards slide in from top (0.5s)
3. Graphs fade in (0.7s)
4. Crash history slides in from bottom (0.9s)

### WebSocket Connect
- Connection status changes color
- Brief pulse animation
- Success sound (optional)

### Crash Detection
- Graph spike turns red
- Large red dot appears
- Crash card animates in at top of history
- Alert banner appears (optional)
- Sound/vibration alert (optional)

### Data Update
- Graphs smoothly transition
- No jarring jumps
- 60 FPS animation
- Hardware accelerated

## 🌐 Responsive Breakpoints

### Mobile (< 768px)
- Single column layout
- Full-width cards
- Stacked graphs
- Touch-optimized buttons
- Large text

### Tablet (768px - 1024px)
- Two-column layout
- Side-by-side graphs
- Medium-sized cards
- Comfortable spacing

### Desktop (> 1024px)
- Multi-column layout
- Wide graphs
- Compact cards
- Maximum information density
- Mouse hover effects

## 🎨 Dark Mode (Future)

Potential dark mode color scheme:

```
Background: #1a1a2e (Dark blue)
Cards: #16213e (Darker blue)
Text: #eee (Light gray)
Accent: #0f3460 (Blue accent)
Graphs: Neon blue/purple
Crashes: Bright red
```

## 📐 Layout Grid

### Mobile Layout
```
┌─────────────┐
│   Header    │ 1 column
├─────────────┤
│   Status    │
├─────────────┤
│   Config    │
├─────────────┤
│  Graph 1    │
├─────────────┤
│  Graph 2    │
├─────────────┤
│   History   │
└─────────────┘
```

### Desktop Layout
```
┌────────────────────────┐
│       Header           │ Full width
├────────────────────────┤
│       Status           │ Full width
├────────────────────────┤
│       Config           │ Full width
├───────────┬────────────┤
│  Graph 1  │  Graph 2   │ 2 columns
├───────────┴────────────┤
│       History          │ Full width
└────────────────────────┘
```

## 🎯 Key UI Principles

### 1. Mobile-First Design
- Works perfectly on small screens
- Touch-friendly buttons (min 44px)
- Clear hierarchy
- Easy navigation

### 2. Real-Time Feedback
- Instant updates (200ms)
- Smooth animations
- Loading indicators
- Success/error messages

### 3. Data Visualization
- Clear, readable graphs
- Color-coded information
- Obvious crash indicators
- Intuitive icons

### 4. User Safety
- Large, clear alerts
- Unmistakable crash indicators
- Easy emergency actions
- Confirmation dialogs

### 5. Professional Polish
- Consistent spacing
- Rounded corners
- Subtle shadows
- Gradient backgrounds
- Icon consistency

## 🔍 Accessibility

### Screen Reader Support
- Semantic HTML
- ARIA labels
- Alt text for icons
- Descriptive button text

### Keyboard Navigation
- Tab through elements
- Enter to activate buttons
- Escape to close modals
- Arrow keys for graphs

### High Contrast
- Clear text on backgrounds
- Visible focus indicators
- Color-blind friendly
- Large click targets

## 📸 Example Use Cases

### Scenario 1: Monitoring During Ride
```
User opens dashboard on phone
  ↓
Sees real-time acceleration/gyroscope
  ↓
Graphs update smoothly as they ride
  ↓
Can glance at phone to check status
  ↓
If crash occurs, sees immediate red spike
```

### Scenario 2: Reviewing After Ride
```
User opens dashboard after ride
  ↓
Scrolls to Crash History section
  ↓
Sees 2 new crashes
  ↓
Clicks first crash
  ↓
Adds note: "Speed bump, not real crash"
  ↓
Clicks View on Map to see location
  ↓
Adjusts thresholds to avoid false alarms
```

### Scenario 3: Configuration
```
User wants to adjust sensitivity
  ↓
Goes to Configuration section
  ↓
Clicks "Get Current Settings"
  ↓
Sees: Acc 2.5g, Gyro 120°/s
  ↓
Changes to: Acc 3.0g, Gyro 150°/s
  ↓
Clicks "Update Thresholds"
  ↓
Gets confirmation: "Settings updated!"
```

## 🎉 Summary

The Smart Helmet Web Dashboard features:

✅ **Beautiful Design**: Modern, gradient background, clean cards
✅ **Real-Time**: Live graphs update 5 times per second
✅ **Mobile-Friendly**: Works perfectly on any device
✅ **Intuitive**: Easy to understand and use
✅ **Informative**: All important data at a glance
✅ **Interactive**: Configure, view, download with ease
✅ **Professional**: Polished animations and transitions

**The dashboard makes safety monitoring accessible, informative, and even enjoyable!**

---

*For technical implementation details, see [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md)*
