# OLED Display Visual Reference

## Regular Status Display

```
┌──────────────────────────────────────────────────────────────┐
│ 0                                                        127  │
│ ┌────────┐                              ┌─────────────┐      │ 0
│ │ ╔════╗ │                              │  ▂ ▄ ▆ █    │      │
│ │ ║████║▌│                              │             │      │
│ │ ╚════╝ │                              │   STRONG    │      │
│ │  100%  │                              └─────────────┘      │
│ └────────┘                                                    │
│                                                               │
│                                                               │
│                                                               │
│                                                               │
│                                                               │
│                                                               │
│   GPS: Connected                                             │
└──────────────────────────────────────────────────────────────┘ 63
  Battery Display (Left)              Network Signal (Right)
  - Icon at (5, 5)                    - Bars at (88, 5)
  - Text at (5, 17)                   - Text at (83, 17)
  
  GPS Status (Bottom)
  - Text at (10, 50)
```

## SMS Notification Display

```
┌──────────────────────────────────────────────────────────────┐
│                                                               │
│                                                               │
│                                                               │
│            ███████  ███   ███  ███████                       │
│            █        █ █ █ █   █                              │
│            ███████  █  █  █   ███████                        │
│                  █  █     █         █                        │
│            ███████  █     █   ███████                        │
│                                                               │
│            ███████  ████  █   █  █████                       │
│            █        █     █ █ █    █                         │
│            ███████  ████  █  ██    █                         │
│                  █  █     █   █    █                         │
│            ███████  ████  █   █    █                         │
│                                                               │
│                    SUCCESS!                                  │
│                                                               │
└──────────────────────────────────────────────────────────────┘
  Large Text: "SMS SENT" at (10, 20), size 2
  Small Text: "SUCCESS!" at (25, 45), size 1
  Duration: 2 seconds, then auto-reset to regular display
```

## Signal Strength Bar Details

### WEAK (RSSI 0-10) - 1 Bar
```
█ ░ ░ ░
```

### MEDIUM (RSSI 11-20) - 3 Bars
```
█ █ █ ░
```

### STRONG (RSSI 21-31) - 4 Bars
```
█ █ █ █
```

### NO SIGNAL (RSSI 0)
```
░ ░ ░ ░
```

## Battery Icon Details

### Full Battery (100%)
```
╔════════════════╗
║████████████████║▌
╚════════════════╝
     100%
```

### Medium Battery (50%)
```
╔════════════════╗
║████████        ║▌
╚════════════════╝
      50%
```

### Low Battery (10%)
```
╔════════════════╗
║█               ║▌
╚════════════════╝
      10%
```

## GPS Status Display Options

### Connecting
```
GPS: Connecting...
```

### Connected
```
GPS: Connected
```

## Display Timing

### Regular Display Updates
- **Update Trigger**: State change (GPS, signal, battery)
- **Frequency**: On-demand only (not every loop)
- **Signal Query**: Every 5 seconds
- **GPS Update**: Every loop (~200ms)

### SMS Notification
- **Trigger**: Successful SMS send after crash detection
- **Display Time**: 2000ms (2 seconds)
- **Auto-Reset**: Returns to regular display automatically

## Screen Coordinates Reference

```
       X-axis (0-127 pixels)
     ┌─────────────────────────┐
  Y  │ (0,0)           (127,0) │
  -  │                         │
  a  │         CENTER          │
  x  │        (64,32)          │
  i  │                         │
  s  │                         │
     │ (0,63)         (127,63) │
  (  └─────────────────────────┘
  0  
  -  
  6  
  3  
     
  p  
  i  
  x  
  e  
  l  
  s  
  )  
```

### Component Positions
- **Battery Icon**: Top-Left
  - Icon: (5, 5) to (25, 15)
  - Text: (5, 17)
  
- **Signal Bars**: Top-Right
  - Bars: (88, 5) to (108, 15)
  - Text: (83, 17)
  
- **GPS Status**: Bottom
  - Text: (10, 50)
  
- **SMS Message**: Center
  - "SMS SENT": (10, 20)
  - "SUCCESS!": (25, 45)

## Font Sizes

### Size 1 (Default)
- Character: 6x8 pixels
- Used for: Battery %, Signal text, GPS status, "SUCCESS!"

### Size 2 (Large)
- Character: 12x16 pixels
- Used for: "SMS SENT" message

## Display Refresh Strategy

### Optimization Approach
```
Previous State           Current State
signalStrength: 25  →   signalStrength: 25   (No update)
gpsConnected: true  →   gpsConnected: false  (Update!)
batteryPercent: 100 →   batteryPercent: 100  (No update)
```

Only updates when any value changes, reducing I2C traffic and power consumption.

## Hardware Wiring Diagram

```
ESP32                    OLED (SSD1306)
┌─────────┐             ┌──────────┐
│         │             │          │
│  GPIO21 ├─────SDA────►│   SDA    │
│  GPIO22 ├─────SCL────►│   SCL    │
│    3.3V ├─────VCC────►│   VCC    │
│     GND ├─────GND────►│   GND    │
│         │             │          │
└─────────┘             └──────────┘
```

## I2C Configuration
- **Address**: 0x3C (default for most SSD1306)
- **Speed**: 100kHz (default), can increase to 400kHz
- **Shared Bus**: MPU6050 also on I2C (address 0x68)

## Memory Usage
- **Display Buffer**: ~1KB (128x64 / 8 bits per pixel)
- **Code Size**: ~3-4KB (functions + libraries)
- **RAM Usage**: ~2KB (buffer + state variables)

## Power Consumption (Typical)
- **Active Display**: 15-20mA @ 3.3V
- **Idle (no updates)**: 10-15mA @ 3.3V
- **Display Off**: < 1mA @ 3.3V

Note: Optimized updates reduce average power consumption.
