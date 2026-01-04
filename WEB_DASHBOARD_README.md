# Smart Helmet Web Dashboard - Complete Guide

## 🌐 Overview

The **Smart Helmet Web Dashboard** is a real-time, mobile-friendly web interface that allows you to monitor your smart helmet's sensor data, view live graphs, manage crash history, and configure detection thresholds from any device (mobile phone, tablet, or laptop).

## ✨ Features

### Real-Time Monitoring
- **Live Acceleration Graph** - Real-time acceleration data plotted as you ride
- **Live Gyroscope Graph** - Real-time rotation data visualization
- **Crash Event Highlighting** - Crash events appear as RED markers on graphs
- **WebSocket Updates** - Instant data updates (5 times per second)
- **System Status** - GPS, battery, crash count, and system state

### Crash Management
- **Crash History View** - See all recorded crashes with details
- **Crash Notes** - Add notes/comments to each crash event
- **Location Links** - Direct Google Maps links for crash locations
- **Download CSV** - Export complete crash history
- **Clear History** - Remove all crash records

### Configuration
- **Configurable Thresholds** - Adjust acceleration and gyroscope sensitivity
- **Impact Time Settings** - Configure crash confirmation time
- **Real-Time Updates** - Changes apply immediately
- **Settings Persistence** - Settings saved between sessions

### Mobile-Friendly Design
- **Responsive Layout** - Works on phones, tablets, and desktops
- **Touch Optimized** - Easy to use on mobile devices
- **Fast Loading** - Lightweight design for quick access
- **Dark Theme Ready** - Professional purple gradient design

## 📋 Requirements

### Hardware
- ESP32 Development Board (with WiFi)
- MPU6050 (Accelerometer + Gyroscope)
- GPS Module (optional for location)
- SIM800L (optional for SMS alerts)
- All components from original smart helmet

### Software
- Arduino IDE with ESP32 support
- Required Arduino Libraries:
  - `Wire` (built-in)
  - `TinyGPSPlus`
  - `Adafruit_GFX`
  - `Adafruit_SSD1306`
  - `SPIFFS` (built-in)
  - `WiFi` (built-in)
  - `WebServer` (built-in)
  - `WebSocketsServer` (install separately)
  - `ArduinoJson` (install separately)

### Network
- WiFi network (2.4GHz recommended)
- Any web browser (Chrome, Firefox, Safari, Edge)
- No internet required (works on local network)

## 🚀 Installation

### Step 1: Install Required Libraries

Open Arduino IDE and install these libraries via Library Manager:

1. **WebSocketsServer** by Markus Sattler
   - Sketch → Include Library → Manage Libraries
   - Search "WebSockets"
   - Install "WebSockets by Markus Sattler"

2. **ArduinoJson** by Benoit Blanchon
   - Search "ArduinoJson"
   - Install "ArduinoJson by Benoit Blanchon"

3. **TinyGPSPlus** by Mikal Hart (if not already installed)

4. **Adafruit GFX** and **Adafruit SSD1306** (if not already installed)

### Step 2: Configure WiFi Credentials

Open `smart-helmet-web-dashboard.ino` and update these lines:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Your WiFi name
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Your WiFi password
```

### Step 3: Configure Emergency Contacts

Update the phone numbers:

```cpp
const char PHONE_NUMBER_1[] = "+1234567890";   // Contact 1
const char PHONE_NUMBER_2[] = "+0987654321";   // Contact 2
const char PHONE_NUMBER_3[] = "+1122334455";   // Contact 3
```

### Step 4: Upload Code

1. Connect ESP32 via USB
2. Select Board: "ESP32 Dev Module"
3. Select Port: Your ESP32's COM port
4. Tools → Partition Scheme → "Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"
5. Click Upload
6. Wait for upload to complete

### Step 5: Find Dashboard IP Address

1. Open Serial Monitor (115200 baud)
2. Wait for system initialization
3. Look for this line:
   ```
   📱 Dashboard URL: http://192.168.1.XXX
   ```
4. Note the IP address
5. The IP also displays on OLED screen

### Step 6: Access Dashboard

1. Ensure your device (phone/laptop) is on the same WiFi network
2. Open web browser
3. Navigate to: `http://[ESP32-IP-ADDRESS]`
4. Dashboard should load immediately

Example: `http://192.168.1.105`

## 📱 Using the Dashboard

### Main Dashboard Interface

#### Header Section
- **Title**: "Smart Helmet Dashboard"
- **Status Bar**:
  - GPS Status: Shows if GPS has fix
  - Battery Level: Current battery percentage
  - Crash Count: Total crashes recorded
  - System Status: "Monitoring" or "CRASH!"
- **Connection Indicator**: Green dot = Connected, Red dot = Disconnected

#### Configuration Card
**Purpose**: Adjust crash detection sensitivity

1. **Acceleration Threshold** (g-force)
   - Default: 2.5g
   - Range: 1.0 - 10.0g
   - Higher = Less sensitive (only severe crashes)
   - Lower = More sensitive (detects lighter impacts)

2. **Gyroscope Threshold** (degrees/second)
   - Default: 120°/s
   - Range: 50 - 300°/s
   - Higher = Less sensitive
   - Lower = More sensitive

3. **Impact Time** (milliseconds)
   - Default: 30ms
   - Range: 10 - 100ms
   - Time impact must persist to confirm crash
   - Higher = Fewer false alarms

**Buttons**:
- **Update Thresholds**: Save new settings
- **Get Current Settings**: View current configuration

#### Live Acceleration Graph
- Shows real-time acceleration magnitude (g-force)
- Blue line = Normal operation
- Red spikes = Crash events
- Updates 5 times per second
- Shows last 50 data points (10 seconds)

#### Live Gyroscope Graph
- Shows real-time rotation rate (degrees/second)
- Purple line = Normal operation
- Red spikes = Crash events
- Updates 5 times per second
- Shows last 50 data points (10 seconds)

#### Crash History Card
**Purpose**: View and manage all recorded crashes

**Each crash entry shows**:
- Crash number and timestamp
- GPS coordinates (Latitude/Longitude)
- Acceleration magnitude at crash
- Gyroscope magnitude at crash
- Battery level at crash
- Optional note/comment

**Actions per crash**:
1. **Add/Edit Note**: Type description and click "Save Note"
   - Example notes: "Pothole on Main St", "False alarm - dropped helmet", "Minor accident at intersection"
2. **View on Map**: Opens Google Maps at crash location
3. **Note indicator**: Blue border = has note, Red border = no note

**Buttons**:
- **Refresh History**: Reload crash list from ESP32
- **Download CSV**: Export all crashes as CSV file
- **Clear All**: Delete all crash records (confirmation required)

## 🎯 Common Use Cases

### Use Case 1: Daily Ride Monitoring

**Scenario**: You want to monitor your helmet during a ride

1. Power on helmet before riding
2. Open dashboard on your phone (bookmark the URL)
3. Keep phone in pocket or mount on bike
4. Dashboard shows live sensor data
5. If crash occurs, you'll see:
   - Red spike on both graphs
   - System Status changes to "CRASH!"
   - Alert on OLED display
   - SMS sent to emergency contacts
6. After ride, review any alerts in Crash History

### Use Case 2: Testing Sensitivity

**Scenario**: You want to adjust crash detection sensitivity

1. Open dashboard
2. Go to Configuration section
3. Click "Get Current Settings" to see defaults
4. Start with current settings:
   - Acc: 2.5g, Gyro: 120°/s, Impact: 30ms
5. Test by simulating impact (drop on soft surface)
6. Too sensitive? Increase thresholds:
   - Acc: 3.0g, Gyro: 150°/s
7. Not sensitive enough? Decrease thresholds:
   - Acc: 2.0g, Gyro: 100°/s
8. Click "Update Thresholds"
9. Test again and observe graphs
10. Repeat until satisfied

### Use Case 3: Reviewing Crash Events

**Scenario**: You had several trips and want to review incidents

1. Open dashboard
2. Scroll to Crash History
3. Click "Refresh History"
4. Review each crash:
   - Check timestamp - when did it happen?
   - Check acceleration/gyroscope - how severe?
   - Check GPS location - where did it happen?
5. Add notes to distinguish crashes:
   - "False alarm - speed bump"
   - "Minor accident - slipped on gravel"
   - "Test crash - calibration"
6. Click "View on Map" to see exact location
7. If needed, download CSV for detailed analysis

### Use Case 4: Exporting Data for Analysis

**Scenario**: You want to analyze crash patterns

1. Open dashboard
2. Go to Crash History section
3. Click "Download CSV"
4. File downloads: `crashes.csv`
5. Open in Excel/Google Sheets/Python:
   ```csv
   Timestamp,Latitude,Longitude,AccMag,GyroMag,Battery%,Note
   123456,23.810332,90.412518,3.45,156.7,85,Pothole on Main St
   ```
6. Create graphs, charts, statistics
7. Identify dangerous locations
8. Share data with authorities/insurance

### Use Case 5: Family Monitoring

**Scenario**: Family member wants to check your safety remotely

**Setup** (one time):
1. Configure port forwarding on your router:
   - Forward external port 8080 to ESP32 IP:80
   - Forward external port 8081 to ESP32 IP:81
2. Find your public IP: `whatismyip.com`
3. Share with family: `http://[PUBLIC-IP]:8080`

**Usage**:
1. Family opens dashboard from anywhere
2. Can see real-time status:
   - Is system online?
   - Current sensor readings
   - Any crash alerts?
3. If crash detected:
   - They see it immediately on dashboard
   - Can view location on map
   - Already received SMS alert
   - Can call for additional help

**Security Note**: For production use, add password protection or VPN.

## 🔧 Technical Details

### WebSocket Protocol

The dashboard uses WebSocket for real-time data streaming:

**Connection**: `ws://[ESP32-IP]:81`

**Message Types**:

1. **Sensor Data** (sent every 200ms):
```json
{
  "type": "sensor",
  "acc": 1.02,
  "gyro": 5.3,
  "time": 123456,
  "crash": false
}
```

2. **Status Update** (sent every 2 seconds):
```json
{
  "type": "status",
  "gps": true,
  "battery": 85,
  "crashes": 3,
  "status": "Monitoring"
}
```

3. **Settings Response**:
```json
{
  "type": "settings",
  "accThreshold": 2.5,
  "gyroThreshold": 120.0,
  "impactTime": 30
}
```

### REST API Endpoints

**GET `/`**
- Returns HTML dashboard page

**GET `/api/settings`**
- Returns current threshold settings
- Response: JSON with accThreshold, gyroThreshold, impactTime

**POST `/api/settings`**
- Updates threshold settings
- Body: JSON with new settings
- Response: `{"status":"ok"}`

**GET `/api/crashes`**
- Returns all crash records
- Response: JSON array of crashes

**POST `/api/crash/note`**
- Saves note for a specific crash
- Body: `{"index": 0, "note": "Your note here"}`
- Response: `{"status":"ok"}`

**GET `/api/download`**
- Downloads crash history as CSV file
- Content-Type: text/csv

**POST `/api/clear`**
- Clears all crash records
- Response: `{"status":"ok"}`

### Data Storage Format

Crashes are stored in SPIFFS at `/crashes.csv`:

```csv
Timestamp,Latitude,Longitude,AccMag,GyroMag,Battery%,Note
123456,23.810332,90.412518,3.45,156.7,85,Pothole on Main St
234567,23.811234,90.413456,4.12,178.3,82,
```

- **Timestamp**: Milliseconds since ESP32 boot
- **Latitude/Longitude**: Decimal degrees (6 decimal places)
- **AccMag**: Acceleration magnitude in g-force (2 decimal places)
- **GyroMag**: Gyroscope magnitude in °/s (1 decimal place)
- **Battery%**: Integer percentage (0-100)
- **Note**: Optional text note (empty if no note)

### Chart Implementation

The dashboard uses **Chart.js 3.9.1** for graphs:

**Features**:
- Line charts with smooth curves
- Dynamic data updates
- Crash event highlighting
- Responsive sizing
- Hardware acceleration

**Crash Highlighting**:
- Normal data: Blue/Purple line, no markers
- Crash data: Red line, large red dot markers
- Makes crashes instantly visible on timeline

### Memory Considerations

**ESP32 Heap Usage**:
- WebServer: ~30KB
- WebSocket: ~20KB
- JSON buffers: ~5KB
- Total overhead: ~55KB
- Plenty of room on ESP32 (520KB RAM)

**SPIFFS Storage**:
- Partition: 1.5MB
- Each crash: ~100 bytes (including note)
- Capacity: ~15,000 crashes
- CSV format: Human-readable and efficient

**Browser Memory**:
- Dashboard keeps last 50 data points in memory
- Old data automatically discarded
- Smooth performance even on low-end phones

## 🛠️ Troubleshooting

### WiFi Connection Issues

**Problem**: ESP32 won't connect to WiFi

**Solutions**:
1. **Check credentials**: Verify SSID and password are correct
2. **Check band**: ESP32 only supports 2.4GHz, not 5GHz WiFi
3. **Check distance**: Move ESP32 closer to router
4. **Check router**: Ensure router is on and broadcasting
5. **Serial output**: Monitor serial for error messages
6. **Restart**: Power cycle ESP32 and try again

**Expected Serial Output**:
```
🔧 Connecting to WiFi...
..........
✅ WiFi Connected
📱 Dashboard URL: http://192.168.1.105
```

### Dashboard Won't Load

**Problem**: Can't access dashboard in browser

**Solutions**:
1. **Check connection**: Ensure device is on same WiFi network as ESP32
2. **Check IP**: Verify you're using correct IP address from serial monitor
3. **Check firewall**: Disable firewall temporarily to test
4. **Try different browser**: Test with Chrome, Firefox, or Safari
5. **Check port**: Ensure port 80 and 81 are not blocked
6. **Ping test**: Try `ping [ESP32-IP]` from command line
7. **Restart browser**: Close and reopen browser
8. **Clear cache**: Clear browser cache and cookies

**Browser Console Check** (F12 → Console):
- Look for WebSocket errors
- Should see: "WebSocket Connected"
- If errors, check IP and port

### WebSocket Won't Connect

**Problem**: Dashboard loads but shows "DISCONNECTED"

**Solutions**:
1. **Check port 81**: WebSocket uses port 81, ensure it's open
2. **Check serial**: Look for "✅ WebSocket started" message
3. **Firewall**: Disable firewall on ESP32 side if applicable
4. **Browser console**: Check for connection errors (F12)
5. **Try different browser**: Some browsers have stricter WebSocket policies
6. **Network restrictions**: Check if network blocks WebSocket connections

**To test WebSocket manually**:
```javascript
// Browser console (F12)
ws = new WebSocket('ws://192.168.1.105:81');
ws.onopen = () => console.log('Connected!');
ws.onerror = (e) => console.error('Error:', e);
```

### Graphs Not Updating

**Problem**: Dashboard loads but graphs don't show data

**Solutions**:
1. **Check WebSocket**: Ensure "CONNECTED" status in top-right
2. **Check serial**: Verify sensor data is being sent every 200ms
3. **Refresh page**: Hard refresh with Ctrl+F5 or Cmd+Shift+R
4. **Check MPU6050**: Ensure accelerometer/gyroscope is working
5. **Check wiring**: Verify MPU6050 connections (SDA, SCL)
6. **Console errors**: Open browser console (F12) and check for errors

**Expected behavior**:
- Graphs should update 5 times per second
- Lines should move smoothly from right to left
- Values should change as helmet moves

### Crash Not Detected

**Problem**: Expected crash but no alert triggered

**Solutions**:
1. **Check thresholds**: May be set too high
   - Try: Acc: 2.0g, Gyro: 80°/s
2. **Check calibration**: Gyroscope must be calibrated at startup
   - Keep helmet still during boot
3. **Watch graphs**: Observe peak values during impact
   - If peaks below thresholds, lower thresholds
4. **Impact time**: May be too long
   - Try: 20ms instead of 30ms
5. **Check MPU6050**: Sensor may be damaged or loose
   - Verify with serial output
6. **Test incrementally**: Start with light taps and increase

### False Crash Alerts

**Problem**: System detects crashes when there are none

**Solutions**:
1. **Increase thresholds**:
   - Acc: 3.0g or higher
   - Gyro: 150°/s or higher
2. **Increase impact time**: 50ms or 100ms
3. **Better mounting**: Secure MPU6050 more firmly
4. **Calibration**: Recalibrate gyroscope (reboot helmet)
5. **Vibration isolation**: Add foam padding around MPU6050
6. **Check graphs**: Observe normal operation peaks and set thresholds above

### Can't Save Notes

**Problem**: Clicking "Save Note" doesn't work

**Solutions**:
1. **Check WebSocket**: Must be connected
2. **Check SPIFFS**: Ensure not full (max ~15,000 crashes)
3. **Check note length**: Very long notes may cause issues
4. **Refresh page**: Reload dashboard and try again
5. **Serial output**: Look for SPIFFS errors
6. **Reformat SPIFFS**: Last resort - will delete all data
   ```cpp
   SPIFFS.format(); // In Arduino code
   ```

### CSV Download Issues

**Problem**: Can't download crash history CSV

**Solutions**:
1. **Check crashes exist**: Must have at least one crash recorded
2. **Check browser**: Allow downloads from local IP addresses
3. **Popup blocker**: Disable popup blocker for dashboard
4. **Try different browser**: Test with Chrome or Firefox
5. **Manual download**: Use direct URL: `http://[ESP32-IP]/api/download`
6. **Check SPIFFS**: File may be corrupted

### Memory Issues

**Problem**: ESP32 crashes or reboots

**Solutions**:
1. **Reduce buffer size**: Change `DATA_BUFFER_SIZE` from 100 to 50
2. **Disable features**: Comment out OLED or other features
3. **Clear SPIFFS**: Old data may be causing issues
4. **Check stack size**: Monitor heap usage in serial output
5. **Power supply**: Ensure adequate power (USB may be insufficient)

### Performance Issues

**Problem**: Dashboard is slow or laggy

**Solutions**:
1. **Reduce data rate**: Change WebSocket update from 200ms to 500ms
2. **Reduce buffer**: Show fewer data points (25 instead of 50)
3. **Close other tabs**: Free up browser memory
4. **Better WiFi**: Move closer to router or use 5GHz (if device supports)
5. **Update browser**: Use latest version of Chrome or Firefox
6. **Disable animations**: Modify chart options to disable transitions

## 📊 Performance Specifications

### Update Rates
- **Sensor Reading**: 200ms (5 Hz)
- **WebSocket Data**: 200ms (5 Hz)
- **Status Update**: 2 seconds (0.5 Hz)
- **Chart Refresh**: Every data point (5 Hz)
- **OLED Update**: 2 seconds (0.5 Hz)

### Latency
- **Sensor to Dashboard**: < 250ms
- **Crash to SMS Alert**: 2-10 seconds (network dependent)
- **Settings Update**: < 100ms
- **Page Load**: < 2 seconds

### Capacity
- **Crash Storage**: ~15,000 crashes (1.5MB SPIFFS)
- **Concurrent Users**: 4-5 simultaneous dashboard viewers
- **Data Buffer**: 50 points × 2 sensors = 100 values in RAM
- **WebSocket Clients**: Up to 5 connections

### Network
- **Bandwidth**: ~2 KB/s per dashboard viewer
- **WiFi Range**: 30-50 meters (typical indoor)
- **Reconnect Time**: 3 seconds (automatic)

## 🔐 Security Considerations

### Current Security
- **No authentication**: Anyone on network can access
- **No encryption**: Data transmitted in plain text
- **Local only**: Not accessible from internet (unless configured)

### Recommended for Production

1. **Add HTTP Authentication**:
```cpp
// In Arduino code
server.on("/", []() {
  if (!server.authenticate("admin", "password")) {
    return server.requestAuthentication();
  }
  server.send_P(200, "text/html", HTML_PAGE);
});
```

2. **Use HTTPS**:
- Replace WebServer with AsyncWebServer
- Add SSL certificate
- Configure HTTPS on port 443

3. **Network Isolation**:
- Keep ESP32 on separate WiFi network
- Use firewall rules
- Implement MAC address filtering

4. **VPN Access**:
- Set up VPN server on home network
- Access dashboard through VPN
- Secure remote access

5. **API Rate Limiting**:
- Limit requests per IP
- Prevent DOS attacks
- Add request throttling

## 🎓 Learning Resources

### Arduino/ESP32
- [ESP32 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)
- [Arduino WebServer Tutorial](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)
- [WebSocket Guide](https://github.com/Links2004/arduinoWebSockets)

### Web Development
- [Chart.js Documentation](https://www.chartjs.org/docs/latest/)
- [WebSocket API](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)
- [Responsive Design](https://www.w3schools.com/css/css_rwd_intro.asp)

### IoT Projects
- [ESP32 IoT Examples](https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi/examples)
- [Real-time Dashboards](https://randomnerdtutorials.com/)

## 🚀 Future Enhancements

### Planned Features
- [ ] Historical data graphs (7-day trends)
- [ ] Heatmap of crash locations
- [ ] Voice alerts through browser
- [ ] Multiple user accounts
- [ ] Cloud data backup
- [ ] Mobile app (iOS/Android)
- [ ] Bluetooth configuration (no WiFi needed)
- [ ] Video recording trigger on crash
- [ ] AI-powered crash prediction
- [ ] Integration with insurance APIs

### Community Contributions
Want to add features? See `CONTRIBUTING.md` for guidelines.

## 📄 Version History

### Version 3.0 (Current)
- ✅ Real-time web dashboard
- ✅ Live sensor graphs
- ✅ Crash event highlighting
- ✅ Crash notes functionality
- ✅ CSV download
- ✅ Configurable thresholds
- ✅ WebSocket streaming
- ✅ Mobile-responsive design

### Version 2.0
- Passive buzzer support
- 3 emergency contacts
- Battery monitoring
- Basic crash logging

### Version 1.0
- Basic crash detection
- GPS tracking
- SMS alerts
- Single contact

## 🆘 Support

### Getting Help

1. **Check Documentation**: Read this guide thoroughly
2. **Serial Monitor**: Check serial output for error messages
3. **Browser Console**: Press F12 and check console for errors
4. **GitHub Issues**: Open issue with:
   - Hardware setup description
   - Serial monitor output
   - Browser console output
   - Steps to reproduce problem
5. **Community Forum**: Ask questions on Arduino or ESP32 forums

### Providing Feedback

Found a bug? Have a suggestion?
- Open GitHub issue
- Include as much detail as possible
- Screenshots are helpful
- Serial logs are valuable

## ⚖️ License

This project is open source and available for educational and personal use.

## 👥 Credits

- **Original Smart Helmet Project Team**
- **Chart.js** - Data visualization
- **Markus Sattler** - WebSocketsServer library
- **Benoit Blanchon** - ArduinoJson library
- **Community Contributors** - Testing and feedback

---

## 📱 Quick Start Checklist

- [ ] Install required libraries
- [ ] Update WiFi credentials
- [ ] Update phone numbers
- [ ] Upload code to ESP32
- [ ] Note IP address from serial monitor
- [ ] Connect phone/laptop to same WiFi
- [ ] Open browser to dashboard URL
- [ ] Verify connection status (green dot)
- [ ] Check graphs are updating
- [ ] Test configuration updates
- [ ] Simulate crash and verify detection
- [ ] Check crash appears in history
- [ ] Add note to crash
- [ ] Download CSV to verify export
- [ ] Bookmark dashboard URL for quick access

**🎉 Congratulations! Your Smart Helmet Web Dashboard is ready!**

---

**Stay Safe and Ride Smart! 🏍️⛑️📱**
