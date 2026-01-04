# Library Installation Guide for Smart Helmet Web Dashboard

## Overview

This guide will help you install all required Arduino libraries for the Smart Helmet Web Dashboard version.

## Required Libraries

### 1. WebSockets by Markus Sattler
**Purpose**: Enables real-time data streaming between ESP32 and web dashboard

**Installation**:
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. In the search box, type: `WebSockets`
4. Find "**WebSockets by Markus Sattler**"
5. Click **Install**
6. Wait for installation to complete

**Version**: 2.3.6 or higher recommended

**Verify Installation**:
```cpp
#include <WebSocketsServer.h>
// If no error, library is installed correctly
```

---

### 2. ArduinoJson by Benoit Blanchon
**Purpose**: JSON serialization/deserialization for API communication

**Installation**:
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. In the search box, type: `ArduinoJson`
4. Find "**ArduinoJson by Benoit Blanchon**"
5. Click **Install**
6. Wait for installation to complete

**Version**: 6.19.0 or higher recommended

**Verify Installation**:
```cpp
#include <ArduinoJson.h>
// If no error, library is installed correctly
```

---

### 3. TinyGPSPlus by Mikal Hart
**Purpose**: GPS data parsing

**Installation**:
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. In the search box, type: `TinyGPSPlus`
4. Find "**TinyGPSPlus by Mikal Hart**"
5. Click **Install**

**Version**: 1.0.3 or higher

**Verify Installation**:
```cpp
#include <TinyGPSPlus.h>
// If no error, library is installed correctly
```

---

### 4. Adafruit GFX Library
**Purpose**: Graphics library for OLED display

**Installation**:
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. In the search box, type: `Adafruit GFX`
4. Find "**Adafruit GFX Library**"
5. Click **Install**
6. Click **Install All** when prompted for dependencies

**Version**: 1.11.0 or higher

---

### 5. Adafruit SSD1306
**Purpose**: Driver for SSD1306 OLED display

**Installation**:
1. Open Arduino IDE
2. Go to **Sketch** → **Include Library** → **Manage Libraries**
3. In the search box, type: `Adafruit SSD1306`
4. Find "**Adafruit SSD1306**"
5. Click **Install**
6. Click **Install All** when prompted for dependencies

**Version**: 2.5.0 or higher

---

### 6. ESP32 Built-in Libraries

These libraries are included with ESP32 board support:

- **WiFi**: WiFi connectivity
- **WebServer**: HTTP server
- **Wire**: I2C communication
- **SPIFFS**: File system

**No manual installation needed** - they come with ESP32 board package.

**To install ESP32 board support** (if not already installed):
1. Go to **File** → **Preferences**
2. In "Additional Board Manager URLs", add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to **Tools** → **Board** → **Boards Manager**
4. Search for "**esp32**"
5. Install "**esp32 by Espressif Systems**"
6. Version 2.0.0 or higher recommended

---

## Installation Order

For smooth installation, install libraries in this order:

1. ESP32 board support (if not installed)
2. WebSockets
3. ArduinoJson
4. TinyGPSPlus
5. Adafruit GFX
6. Adafruit SSD1306

---

## Common Installation Issues

### Issue 1: Library Manager Not Loading

**Solution**:
- Check internet connection
- Restart Arduino IDE
- Try **File** → **Preferences** → Click "Library Manager verbose output"
- Check firewall settings

---

### Issue 2: "Library Not Found" Error

**Symptoms**:
```
fatal error: WebSocketsServer.h: No such file or directory
```

**Solution**:
1. Verify library is installed: **Sketch** → **Include Library** → Should see library listed
2. Try manual installation:
   - Download ZIP from GitHub
   - **Sketch** → **Include Library** → **Add .ZIP Library**
3. Check library folder: Documents/Arduino/libraries/
4. Restart Arduino IDE

---

### Issue 3: Version Conflicts

**Symptoms**:
```
error: 'class WebSocketsServer' has no member named 'begin'
```

**Solution**:
- Update to latest version of library
- Remove old versions from libraries folder
- Reinstall from Library Manager

---

### Issue 4: ESP32 Board Not Found

**Symptoms**:
- Can't select ESP32 Dev Module in boards menu

**Solution**:
1. Verify board manager URL is correct
2. Go to **Tools** → **Board** → **Boards Manager**
3. Search "esp32" and install if missing
4. Restart Arduino IDE
5. Check **Tools** → **Board** → Should see "ESP32 Dev Module"

---

## Manual Installation (Alternative Method)

If Library Manager doesn't work, install manually:

### WebSockets Library
1. Download: https://github.com/Links2004/arduinoWebSockets/releases
2. Extract ZIP file
3. Copy folder to: Documents/Arduino/libraries/
4. Rename folder to: "WebSockets"
5. Restart Arduino IDE

### ArduinoJson Library
1. Download: https://github.com/bblanchon/ArduinoJson/releases
2. Extract ZIP file
3. Copy folder to: Documents/Arduino/libraries/
4. Rename folder to: "ArduinoJson"
5. Restart Arduino IDE

---

## Verification Script

Create a new sketch and paste this code to verify all libraries:

```cpp
// Library Verification Script
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Library Verification ===");
  
  Serial.println("✅ WiFi");
  Serial.println("✅ WebServer");
  Serial.println("✅ WebSocketsServer");
  Serial.println("✅ ArduinoJson");
  Serial.println("✅ Wire");
  Serial.println("✅ TinyGPSPlus");
  Serial.println("✅ Adafruit_GFX");
  Serial.println("✅ Adafruit_SSD1306");
  Serial.println("✅ SPIFFS");
  
  Serial.println("\nAll libraries verified!");
  Serial.println("You're ready to upload smart-helmet-web-dashboard.ino");
}

void loop() {
  // Nothing
}
```

**Expected Output**:
```
=== Library Verification ===
✅ WiFi
✅ WebServer
✅ WebSocketsServer
✅ ArduinoJson
✅ Wire
✅ TinyGPSPlus
✅ Adafruit_GFX
✅ Adafruit_SSD1306
✅ SPIFFS

All libraries verified!
You're ready to upload smart-helmet-web-dashboard.ino
```

If compilation succeeds without errors, all libraries are installed correctly!

---

## Next Steps

After installing all libraries:

1. ✅ Open `smart-helmet-web-dashboard.ino`
2. ✅ Configure WiFi credentials
3. ✅ Configure phone numbers
4. ✅ Select Board: "ESP32 Dev Module"
5. ✅ Select Partition: "Default 4MB with spiffs"
6. ✅ Upload code
7. ✅ Open Serial Monitor (115200 baud)
8. ✅ Note dashboard IP address
9. ✅ Open browser to IP address
10. ✅ Start monitoring!

---

## Support

If you encounter issues:
1. Check Arduino IDE version (1.8.13+ or 2.0+)
2. Check ESP32 board support version (2.0.0+)
3. Verify internet connection
4. Try restarting Arduino IDE
5. Check antivirus/firewall settings
6. See WEB_DASHBOARD_README.md for troubleshooting
7. Open GitHub issue with error details

---

## Library Versions Tested

This project has been tested with:

| Library | Version | Status |
|---------|---------|--------|
| ESP32 Core | 2.0.11 | ✅ Working |
| WebSockets | 2.3.6 | ✅ Working |
| ArduinoJson | 6.21.3 | ✅ Working |
| TinyGPSPlus | 1.0.3 | ✅ Working |
| Adafruit GFX | 1.11.3 | ✅ Working |
| Adafruit SSD1306 | 2.5.7 | ✅ Working |

Newer versions should also work, but these are confirmed working.

---

**Happy Coding! 🚀**
