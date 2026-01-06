# Quick Start Guide - new-mpu-web.ino

## 🚀 What's New?

This is an enhanced version of `mpu-test-web-dashboard.ino` with **real Dhaka timezone support**!

## ✨ Key Enhancements

### 1. **Real Timezone Support 🇧🇩**
- Automatic time synchronization with NTP servers
- All timestamps displayed in **Dhaka, Bangladesh time (GMT+6)**
- Live clock display on dashboard
- Human-readable time in crash history

### 2. **Enhanced Crash Visualization 🔴**
- Crash points marked with **large RED dots** on graphs
- Point radius: 10 (vs 8 in original)
- Hover tooltips show "🚨 CRASH DETECTED"
- Better visual distinction

### 3. **Improved CSV Export 📊**
- New "Dhaka Time" column added
- Format: `YYYY-MM-DD HH:MM:SS`
- Makes data analysis easier with actual timestamps

## 🔧 Installation

### Extra Library Required

In addition to the libraries needed for the original version, install:

**NTPClient** by Fabrice Weinberg
- Open Arduino IDE
- Go to Sketch → Include Library → Manage Libraries
- Search for "NTPClient"
- Install version 3.2.0 or later

### All Required Libraries

1. **WebSockets** by Markus Sattler
2. **ArduinoJson** by Benoit Blanchon (v6.x)
3. **NTPClient** by Fabrice Weinberg ⭐ NEW
4. **Wire** (built-in)

## 🎯 Quick Setup

### 1. WiFi Configuration

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

### 2. Upload & Check Time

Open Serial Monitor (115200 baud) and look for:

```
✅ Time synchronized!
🇧🇩 Dhaka Time: 2024-01-06 19:30:45
```

### 3. Access Dashboard

Open browser to the IP address shown, you'll see:
- 🇧🇩 Dhaka, Bangladesh (GMT+6) badge
- Live current time display
- All crash timestamps in Dhaka time

## 📊 Dashboard Features

### Live Time Display
```
🕐 2024-01-06 19:30:45
```
Updates every second!

### Crash History with Time
```
🚨 CRASH DETECTED
Time (Dhaka): 2024-01-06 19:30:47
Acceleration: 3.87 g
Gyroscope: 145.67 °/s
```

### Enhanced Graphs
- Blue line: Acceleration
- Green line: Gyroscope
- 🔴 Large RED dots: Crash points
- Hover to see crash label

## 📥 CSV Download Format

```csv
Timestamp (ms),Dhaka Time,Acceleration (g),Gyroscope (deg/s),Crash Detected
1234,2024-01-06 19:30:45,1.02,3.45,NO
1634,2024-01-06 19:30:47,3.87,145.67,YES
```

Notice the new "Dhaka Time" column! 🎉

## 🔍 Comparison Table

| Feature | Original | new-mpu-web.ino |
|---------|----------|-----------------|
| MPU6050 Support | ✅ | ✅ |
| Web Dashboard | ✅ | ✅ |
| Crash Detection | ✅ | ✅ |
| RED Markers | ✅ Small | ✅ **Large** |
| WebSocket Updates | ✅ | ✅ |
| CSV Download | ✅ | ✅ **+ Time** |
| Timezone Support | ❌ | ✅ **Dhaka GMT+6** |
| NTP Sync | ❌ | ✅ **Auto** |
| Live Clock | ❌ | ✅ **Yes** |
| Human Time | ❌ | ✅ **YYYY-MM-DD HH:MM:SS** |

## ⚠️ Important Notes

### Internet Required
The ESP32 must have internet access through WiFi to sync time with NTP servers. Without internet:
- Time will show "Time not synced"
- System continues working normally
- Only millisecond timestamps available

### WiFi Requirements
- 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Internet access for NTP
- Same network for phone/computer access

### Time Sync
- Takes 5-10 seconds on first boot
- Updates every 60 seconds automatically
- Uses pool.ntp.org server
- GMT+6 offset applied automatically

## 🎓 Usage Tips

### 1. Verify Time Sync
Always check Serial Monitor to confirm time is synced:
```
✅ Time synchronized!
🇧🇩 Dhaka Time: 2024-01-06 19:30:45
```

### 2. Check Dashboard Time
Look for the live clock on dashboard header:
```
🕐 2024-01-06 19:30:45
```

### 3. Test Crash Detection
When you trigger a crash, check:
- ✅ Large RED dots appear on graphs
- ✅ Crash history shows Dhaka time
- ✅ Serial monitor shows timestamp

### 4. Download Data
The CSV now has human-readable Dhaka time:
- Easier to correlate with real events
- No need to convert milliseconds
- Ready for analysis in Excel

## 🚨 Troubleshooting

### "Time not synced" Error

**Possible causes:**
1. No internet access through WiFi
2. Firewall blocking NTP (UDP port 123)
3. NTP server unreachable

**Solutions:**
- Verify WiFi has internet
- Wait 20-30 seconds after boot
- Restart ESP32
- Check router firewall settings

### Time is Wrong

**Check:**
1. Verify UTC_OFFSET_SECONDS = 21600 (GMT+6)
2. Ensure NTP sync completed successfully
3. Check system time on your computer

### Crashes Not Showing in RED

**Check:**
1. WebSocket connection (top-right status)
2. Refresh browser page
3. Check Serial Monitor for crash events
4. Adjust thresholds if needed

## 📖 Full Documentation

For complete details, see:
- **NEW_MPU_WEB_README.md** - Full documentation
- **MPU_TEST_README.md** - Original version docs

## 🎯 When to Use Which Version?

### Use `mpu-test-web-dashboard.ino` if:
- You don't need timezone support
- No internet access available
- Want simpler code
- Testing in controlled lab environment

### Use `new-mpu-web.ino` if:
- You need real Dhaka time ⭐
- Have internet access
- Want human-readable timestamps
- Recording data for later analysis
- Correlating with real-world events

## 🔄 Migration from Original

If you're currently using `mpu-test-web-dashboard.ino`:

1. **Install NTPClient library**
2. **Upload new-mpu-web.ino**
3. **Verify time sync** in Serial Monitor
4. **Test on dashboard** - check clock display
5. **Done!** All features preserved + timezone

## 💡 Pro Tips

### Tip 1: Verify Time Before Testing
Always confirm time is correct before starting tests:
```
Serial.println(getDhakaTime());
```

### Tip 2: Use Time for Analysis
When reviewing CSV data, the Dhaka time column helps you:
- Remember when each test occurred
- Correlate with video footage
- Compare different test sessions

### Tip 3: Network Reliability
For best results:
- Use stable WiFi connection
- Ensure router has internet
- Place ESP32 near router during sync

### Tip 4: Timezone Customization
To change timezone (e.g., for different location):
```cpp
// For GMT+8 (e.g., Singapore)
const long UTC_OFFSET_SECONDS = 28800;

// For GMT-5 (e.g., EST)
const long UTC_OFFSET_SECONDS = -18000;
```

## 🎉 Summary

**new-mpu-web.ino** = **mpu-test-web-dashboard.ino** + **Real Dhaka Timezone**

Everything you loved about the original, plus:
- 🕐 Real time display
- 🇧🇩 Dhaka timezone (GMT+6)
- 🔴 Enhanced crash visualization
- 📊 Better CSV exports
- 🌐 NTP synchronization

Perfect for real-world testing where you need to know *exactly when* crashes occurred!

---

**Ready to test with real time? Upload and enjoy! 🚀🇧🇩**
