# OLED Display Implementation Guide

## Overview
This document describes the OLED display functionality implemented for the Smart Helmet system. The display shows real-time status information including battery level, network signal strength, GPS connection status, and crash/SMS notifications.

## Hardware Requirements
- **OLED Display**: SSD1306-based OLED (128x64 pixels, I2C interface)
- **I2C Address**: 0x3C (default for most SSD1306 displays)
- **Connections**:
  - SDA → GPIO 21 (ESP32)
  - SCL → GPIO 22 (ESP32)
  - VCC → 3.3V
  - GND → GND

## Required Libraries
```cpp
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
```

Install via Arduino Library Manager:
1. Adafruit SSD1306
2. Adafruit GFX Library

## Features Implemented

### 1. Battery Status Display (Left Side)
- **Visual**: Battery icon with fill level
- **Text**: Percentage (e.g., "100%")
- **Location**: Top-left corner (x:5, y:5)
- **Icon Size**: 20x10 pixels with battery tip
- **Default Value**: 100% (can be connected to battery sensor)

### 2. Network Signal Strength (Right Side)
- **Query Method**: AT+CSQ command to SIM800L
- **Query Interval**: Every 5 seconds
- **Signal Mapping**:
  - **0-10**: WEAK (1 bar displayed)
  - **11-20**: MEDIUM (3 bars displayed)
  - **21-31**: STRONG (4 bars displayed)
- **Visual**: 4 signal bars (filled/outlined based on strength)
- **Text Labels**: "WEAK", "MEDIUM", "STRONG", or "NO SIG"
- **Location**: Top-right corner (x:88, y:5)

### 3. GPS Connection Status (Bottom)
- **States**:
  - `GPS: Connecting...` - While waiting for GPS fix
  - `GPS: Connected` - When `gps.location.isValid()` returns true
- **Location**: Bottom of screen (x:10, y:50)
- **Font Size**: 1 (standard text)

### 4. SMS Notification Display
- **Trigger**: After successful SMS transmission
- **Display**: 
  - Large text: "SMS SENT" (size 2)
  - Small text: "SUCCESS!" (size 1)
- **Duration**: 2 seconds
- **Behavior**: Automatically returns to regular status display

## Function Descriptions

### `querySignalStrength()`
Queries the SIM800L module for signal strength using AT+CSQ command.
- **Returns**: Integer (0-31) representing RSSI value
- **Timeout**: 2 seconds
- **Parser**: Extracts RSSI from "+CSQ: <rssi>,<ber>" response

### `drawBatteryIcon(int x, int y, int percent)`
Renders battery icon with percentage text.
- **Parameters**: 
  - x, y: Top-left position
  - percent: Battery level (0-100)
- **Visual Elements**:
  - Battery outline (18x10 pixels)
  - Battery tip (2x4 pixels)
  - Fill bar (proportional to percentage)
  - Percentage text below icon

### `drawSignalBars(int x, int y, int strength)`
Renders signal strength bars and text.
- **Parameters**:
  - x, y: Top-left position
  - strength: RSSI value (0-31)
- **Visual Elements**:
  - 4 bars with heights: 4, 6, 8, 10 pixels
  - Filled bars for active signal
  - Outlined bars for inactive
  - Signal quality text

### `updateOLEDDisplay()`
Main display update function called in loop.
- **Modes**:
  1. **SMS Mode**: Shows "SMS SENT" message
  2. **Regular Mode**: Shows battery, network, GPS status
- **Call Frequency**: Every loop iteration (~200ms) when not showing SMS

## Display Layout

```
+---------------------------+
|  [Battery]      [Signal]  |
|    100%         STRONG    |
|   [====]        ████      |
|                           |
|                           |
|                           |
|   GPS: Connected          |
+---------------------------+
```

### SMS Notification Layout
```
+---------------------------+
|                           |
|                           |
|      SMS SENT             |
|                           |
|       SUCCESS!            |
|                           |
+---------------------------+
```

## Integration Points

### In `setup()`:
```cpp
// Initialize OLED
if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println(F("SSD1306 allocation failed"));
  for(;;); // Don't proceed
}
display.clearDisplay();
display.setTextColor(SSD1306_WHITE);
// ... initialization messages ...
updateOLEDDisplay();
```

### In `loop()`:
```cpp
// Update GPS status
gpsConnected = gps.location.isValid();

// Query signal strength every 5 seconds
static unsigned long lastSignalQuery = 0;
if (millis() - lastSignalQuery >= 5000) {
  signalStrength = querySignalStrength();
  lastSignalQuery = millis();
}

// Show SMS message after successful send
if (sendSMS(...)) {
  smsSent = true;
  showingSMSMessage = true;
  smsDisplayTime = millis();
  updateOLEDDisplay();
}

// Clear SMS message after 2 seconds
if (showingSMSMessage && (millis() - smsDisplayTime >= 2000)) {
  showingSMSMessage = false;
  updateOLEDDisplay();
}

// Regular display update
if (!showingSMSMessage) {
  updateOLEDDisplay();
}
```

## Global Variables

```cpp
int signalStrength = 0;           // RSSI value (0-31)
bool gpsConnected = false;         // GPS fix status
int batteryPercent = 100;          // Battery level
unsigned long smsDisplayTime = 0;  // Timestamp for SMS display
bool showingSMSMessage = false;    // SMS display flag
```

## Timing Considerations

1. **Signal Query**: Every 5 seconds to avoid overloading SIM800L
2. **Display Update**: Every loop iteration (~200ms)
3. **SMS Display**: 2 seconds duration
4. **Loop Delay**: 200ms base delay

## Customization Options

### Battery Monitoring
To connect a real battery sensor:
```cpp
// In loop(), read battery voltage
float voltage = analogRead(BATTERY_PIN) * (3.3 / 4095.0) * 2; // Adjust divider
batteryPercent = map(voltage * 100, 330, 420, 0, 100); // Map 3.3V-4.2V to 0-100%
batteryPercent = constrain(batteryPercent, 0, 100);
```

### Display Position Adjustment
Modify position constants in function calls:
```cpp
drawBatteryIcon(5, 5, batteryPercent);    // Left side
drawSignalBars(88, 5, signalStrength);    // Right side
display.setCursor(10, 50);                // GPS status
```

### Signal Strength Thresholds
Adjust mapping in `drawSignalBars()`:
```cpp
if (strength >= 25) numBars = 4;      // Very Strong
else if (strength >= 18) numBars = 3; // Strong
else if (strength >= 10) numBars = 2; // Medium
else if (strength > 0) numBars = 1;   // Weak
```

## Troubleshooting

### Display Not Working
1. Check I2C address (use I2C scanner: 0x3C or 0x3D)
2. Verify SDA/SCL connections (GPIO 21/22)
3. Check power supply (3.3V or 5V depending on module)
4. Verify library installation

### Signal Strength Always Zero
1. Check SIM800L power and antenna
2. Verify UART connections (GPIO 26/27)
3. Ensure SIM card is inserted and network registered
4. Check AT+CSQ response in serial monitor

### GPS Status Not Updating
1. Verify GPS module connections (GPIO 16/17)
2. Ensure GPS has clear sky view
3. Wait for initial GPS fix (may take minutes)
4. Check GPS UART baud rate (9600)

### SMS Message Not Displaying
1. Verify SMS is actually being sent successfully
2. Check `showingSMSMessage` flag is set
3. Ensure `smsDisplayTime` is recorded
4. Verify 2-second timer logic

## Performance Notes

- **Memory Usage**: ~3-4KB for display buffer
- **I2C Speed**: Default 100kHz (can increase to 400kHz)
- **Update Rate**: ~5 FPS (limited by 200ms loop delay)
- **Signal Query**: Minimal impact (~100ms every 5 seconds)

## Future Enhancements

1. **Battery Icon Animation**: Charging indicator
2. **Signal Strength Graph**: Historical signal quality
3. **GPS Satellite Count**: Number of satellites in view
4. **Speed Display**: Current speed from GPS
5. **Time Display**: Current time from GPS
6. **Temperature Display**: From MPU6050 sensor
7. **Menu System**: Settings and configuration
8. **Low Battery Warning**: Flash icon below threshold

## Code Quality

- ✅ Modular functions for each display element
- ✅ Clear variable naming
- ✅ Commented code sections
- ✅ Configurable timing constants
- ✅ Error handling for OLED initialization
- ✅ Non-blocking display updates
- ✅ Minimal impact on main loop performance
