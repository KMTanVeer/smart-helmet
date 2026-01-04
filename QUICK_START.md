# Quick Start Guide - Smart Helmet Web Dashboard

Get your Smart Helmet Web Dashboard running in 10 minutes!

## ⚡ Prerequisites

- [ ] ESP32 Development Board
- [ ] MPU6050 sensor (accelerometer + gyroscope)
- [ ] WiFi network (2.4GHz)
- [ ] Arduino IDE installed
- [ ] USB cable for ESP32

**Optional**: GPS module, SIM800L for SMS, OLED display, buzzer, battery monitor

## 🚀 5-Step Installation

### Step 1: Install Libraries (5 minutes)

Open Arduino IDE and install these libraries via **Sketch → Include Library → Manage Libraries**:

1. Search "**WebSockets**" → Install "**WebSockets by Markus Sattler**"
2. Search "**ArduinoJson**" → Install "**ArduinoJson by Benoit Blanchon**"
3. Search "**TinyGPSPlus**" → Install if using GPS
4. Search "**Adafruit GFX**" → Install if using OLED
5. Search "**Adafruit SSD1306**" → Install if using OLED

Need help? See [LIBRARY_INSTALLATION.md](LIBRARY_INSTALLATION.md)

### Step 2: Configure WiFi (1 minute)

Open `smart-helmet-web-dashboard.ino` and edit these lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

Example:
```cpp
const char* WIFI_SSID = "Home_WiFi";
const char* WIFI_PASSWORD = "mypassword123";
```

### Step 3: Configure Board (1 minute)

In Arduino IDE:
1. **Tools → Board → ESP32 Dev Module**
2. **Tools → Partition Scheme → Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)**
3. **Tools → Upload Speed → 115200**
4. **Tools → Port → Select your ESP32's COM port**

### Step 4: Upload Code (2 minutes)

1. Connect ESP32 to computer via USB
2. Click **Upload** button (→) in Arduino IDE
3. Wait for "Hard resetting via RTS pin..."
4. Success! Code is uploaded

### Step 5: Find Dashboard URL (1 minute)

1. Open **Serial Monitor** (🔍 icon or Ctrl+Shift+M)
2. Set baud rate to **115200**
3. Look for this line:
   ```
   📱 Dashboard URL: http://192.168.1.105
   ```
4. Copy the IP address

## 🎉 You're Done!

### Access Dashboard

1. Ensure your phone/laptop is on the **same WiFi network**
2. Open web browser (Chrome, Firefox, Safari)
3. Go to: `http://[YOUR-ESP32-IP]`
4. Dashboard should load immediately!

Example: `http://192.168.1.105`

### What You'll See

- **Connection Status**: Green dot = Connected
- **Live Graphs**: Blue/purple lines updating in real-time
- **System Status**: GPS, Battery, Crash Count
- **Configuration**: Adjust thresholds
- **Crash History**: Empty initially

### Test It!

1. Pick up ESP32 and shake it gently
2. Watch graphs respond to movement
3. Check values change in real-time
4. Try adjusting thresholds

## 🔧 Minimal Setup (Without Optional Components)

You can run the dashboard with just **ESP32 + MPU6050**:

### Wiring (Minimum)

```
MPU6050 → ESP32
------------------
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22
```

That's it! Other components are optional.

### Code Adjustments

If you don't have optional components, the code will still work:
- **No GPS**: System works, but location shows 0,0
- **No SIM800L**: System works, but no SMS alerts
- **No OLED**: System works, dashboard shows all info
- **No Buzzer**: System works, no audio alerts
- **No Battery**: System works, battery shows 100%

Everything still functions, just with reduced features.

## 📱 Mobile Access

### On Phone

1. Connect phone to same WiFi as ESP32
2. Open browser (Chrome, Safari)
3. Go to: `http://[ESP32-IP]`
4. **Bookmark** for easy access!
5. Works like an app

### Add to Home Screen

**iPhone**:
1. Open dashboard in Safari
2. Tap Share → Add to Home Screen
3. Name it "Smart Helmet"
4. Now it's like an app icon!

**Android**:
1. Open dashboard in Chrome
2. Tap Menu (⋮) → Add to Home screen
3. Name it "Smart Helmet"
4. Now it's like an app icon!

## 🎯 Common First-Time Issues

### "Can't find dashboard"

**Check**:
- [ ] Phone on same WiFi as ESP32?
- [ ] Using correct IP from Serial Monitor?
- [ ] ESP32 connected to WiFi? (Check Serial Monitor)

### "WebSocket disconnected"

**Solution**:
- Refresh page (F5 or pull down)
- Wait 3 seconds for auto-reconnect
- Check Serial Monitor for errors

### "No data on graphs"

**Check**:
- [ ] WebSocket connected? (Green dot top-right)
- [ ] MPU6050 wired correctly?
- [ ] Serial Monitor shows sensor readings?

## 🆘 Need Help?

1. **Read full guide**: [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md)
2. **Check wiring**: Verify all connections
3. **Serial Monitor**: Look for error messages
4. **Browser Console**: Press F12, check for errors
5. **Ask for help**: Open GitHub issue

## 📚 What's Next?

### Configure Detection

1. Open Configuration section
2. Test current thresholds:
   - Shake helmet gently
   - Check graph peaks
3. Adjust if needed:
   - Too sensitive? Increase thresholds
   - Not sensitive? Decrease thresholds
4. Click "Update Thresholds"

### Add Emergency Contacts

Edit these lines in code:

```cpp
const char PHONE_NUMBER_1[] = "+1234567890";   // Your number
const char PHONE_NUMBER_2[] = "+0987654321";   // Family
const char PHONE_NUMBER_3[] = "+1122334455";   // Friend
```

Include country code! Format: `+[country][number]`

### Test Crash Detection

1. Set conservative thresholds first:
   - Acc: 2.5g
   - Gyro: 120°/s
   - Impact: 30ms
2. Drop helmet on pillow/cushion
3. Watch for crash alert
4. Check Crash History
5. Add note: "Test crash"
6. Adjust thresholds as needed

### Install Full Hardware

When ready for production:
1. Add GPS module (location tracking)
2. Add SIM800L (SMS alerts)
3. Add OLED display (status on helmet)
4. Add buzzer (audio alerts)
5. Add battery monitor (power status)
6. Mount in helmet

See [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md) for wiring details.

## 🎓 Learning Path

**Beginner**:
1. ✅ Get dashboard working (this guide)
2. ✅ Test with ESP32 + MPU6050 only
3. ✅ Learn to use web interface
4. ✅ Adjust thresholds

**Intermediate**:
1. ✅ Add GPS for location
2. ✅ Add OLED for display
3. ✅ Add battery monitoring
4. ✅ Test crash detection outdoors

**Advanced**:
1. ✅ Add SIM800L for SMS
2. ✅ Configure 3 contacts
3. ✅ Mount in helmet
4. ✅ Real-world testing
5. ✅ Fine-tune for your riding style

## 📊 System Status Reference

### Connection Status Indicator
- **Green dot**: WebSocket connected, data streaming
- **Red dot**: Disconnected, trying to reconnect

### GPS Status
- **Connected**: GPS has fix, location available
- **Searching...**: GPS module present, acquiring satellites
- **--**: GPS not connected or not configured

### System Status
- **Monitoring**: Normal operation, watching for crashes
- **CRASH!**: Accident detected, alerts triggered

### Battery
- **0-100%**: Current battery level
- **Green**: >50%
- **Orange**: 20-50%
- **Red**: <20%
- **--**: Battery monitor not configured

## 🔖 Bookmarks

Save these for quick reference:

- Dashboard: `http://[YOUR-ESP32-IP]`
- Download CSV: `http://[YOUR-ESP32-IP]/api/download`
- API Settings: `http://[YOUR-ESP32-IP]/api/settings`

## ✅ Checklist

Before first ride:

- [ ] Dashboard accessible from phone
- [ ] WebSocket connected (green dot)
- [ ] Graphs updating in real-time
- [ ] GPS connected (if installed)
- [ ] Battery showing percentage (if installed)
- [ ] Test crash detection works
- [ ] Emergency contacts configured (if SMS enabled)
- [ ] Thresholds adjusted for your needs
- [ ] Dashboard bookmarked on phone
- [ ] Helmet securely mounted

## 🎉 Success!

You now have a working Smart Helmet Web Dashboard!

**Key Features**:
- ✅ Real-time monitoring
- ✅ Live sensor graphs
- ✅ Crash detection
- ✅ Mobile access
- ✅ Configurable settings

**Enjoy your ride! Stay safe! 🏍️⛑️📱**

---

**Next Steps**:
- Read full documentation: [WEB_DASHBOARD_README.md](WEB_DASHBOARD_README.md)
- Compare versions: See "Version Comparison" in [README.md](README.md)
- Learn about crash data: [CRASH_DATA_GUIDE.md](CRASH_DATA_GUIDE.md)

**Questions?** Open an issue on GitHub!
