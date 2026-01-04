/* 
 * =====================================================
 * SMART HELMET - WEB DASHBOARD VERSION
 * =====================================================
 * 
 * Features:
 * - Real-time crash detection (accelerometer + gyroscope)
 * - Configurable thresholds
 * - Time-based crash confirmation
 * - Crash data logging in SPIFFS (CSV)
 * - Mobile-friendly web dashboard
 * - Live acceleration and gyroscope graphs
 * - Crash events highlighted in red
 * - Crash notes functionality
 * - Download crash history
 * - Real-time data streaming via WebSocket
 * 
 * HARDWARE COMPONENTS:
 * --------------------
 * 1. ESP32 Development Board
 * 2. MPU6050 - Gyroscope & Accelerometer
 * 3. Ublox Neo-8M GPS Module
 * 4. SIM800L GSM/GPRS Module
 * 5. Passive Buzzer
 * 6. Push Button (Cancel)
 * 7. OLED Display SSD1306
 * 8. Battery Voltage Sensor
 * 
 * WIRING: See README.md
 * 
 * WEB DASHBOARD ACCESS:
 * ---------------------
 * 1. Connect ESP32 to WiFi
 * 2. Open browser to http://[ESP32-IP]
 * 3. View real-time graphs
 * 4. Download crash history
 * 5. Add notes to crashes
 * 
 * Author: Smart Helmet Web Dashboard Project
 * Version: 3.0
 */

#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

/* ================= WIFI CONFIGURATION ================= */
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ CHANGE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ CHANGE THIS

/* ================= OLED DISPLAY ================= */
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/* ================= MPU6050 ================= */
#define MPU_ADDR 0x68
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

/* ================= GPS ================= */
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);
#define GPS_RX 16
#define GPS_TX 17

/* ================= SIM800L ================= */
HardwareSerial sim800(1);
#define SIM_RX 26
#define SIM_TX 27

const char PHONE_NUMBER_1[] = "+1234567890";   // ⚠️ CHANGE TO CONTACT 1
const char PHONE_NUMBER_2[] = "+0987654321";   // ⚠️ CHANGE TO CONTACT 2
const char PHONE_NUMBER_3[] = "+1122334455";   // ⚠️ CHANGE TO CONTACT 3

/* ================= PASSIVE BUZZER & BUTTON ================= */
#define BUZZER_PIN 25
#define CANCEL_BTN 33

/* ================= BATTERY MONITORING ================= */
#define BATTERY_PIN 34
#define BATTERY_FULL_VOLTAGE 4.2
#define BATTERY_EMPTY_VOLTAGE 3.3
#define VOLTAGE_DIVIDER_RATIO 2.0

/* ================= THRESHOLDS (CONFIGURABLE VIA WEB) ================= */
float ACC_THRESHOLD = 2.5;      // g-force
float GYRO_THRESHOLD = 120.0;   // degrees per second
int IMPACT_TIME_MS = 30;        // milliseconds

/* ================= TONE DEFINITIONS ================= */
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

/* ================= STATE VARIABLES ================= */
bool crashDetected = false;
bool smsSent = false;
unsigned long impactStart = 0;
unsigned long crashTime = 0;
bool warningTonePlaying = false;
unsigned long warningToneStart = 0;
bool beepTonePlaying = false;
unsigned long beepToneStart = 0;

/* ================= WEB SERVER & WEBSOCKET ================= */
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

/* ================= SENSOR DATA BUFFERS (FOR GRAPHS) ================= */
#define DATA_BUFFER_SIZE 100
float accBuffer[DATA_BUFFER_SIZE];
float gyroBuffer[DATA_BUFFER_SIZE];
unsigned long timeBuffer[DATA_BUFFER_SIZE];
bool crashBuffer[DATA_BUFFER_SIZE];
int bufferIndex = 0;
unsigned long lastDataUpdate = 0;

/* ================= OLED DISPLAY STATE ================= */
int signalStrength = 0;
bool gpsConnected = false;
int batteryPercent = 100;
bool displayAvailable = false;
String webDashboardIP = "";

/* ================= CRASH DATA STRUCTURE ================= */
int crashLogCount = 0;

/* ================= HTML WEB DASHBOARD ================= */
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Helmet Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js@3.9.1/dist/chart.min.js"></script>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: #333;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            text-align: center;
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .status {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 10px;
            margin-top: 15px;
        }
        .status-item {
            background: #f0f4ff;
            padding: 10px 20px;
            border-radius: 10px;
            font-weight: bold;
        }
        .status-ok { color: #22c55e; }
        .status-warn { color: #f59e0b; }
        .status-danger { color: #ef4444; }
        .card {
            background: white;
            padding: 20px;
            border-radius: 15px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        .card h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.3em;
        }
        .chart-container {
            position: relative;
            height: 300px;
            margin-bottom: 20px;
        }
        .controls {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 15px;
        }
        input, button, select {
            padding: 10px 15px;
            border-radius: 8px;
            border: 2px solid #667eea;
            font-size: 14px;
        }
        button {
            background: #667eea;
            color: white;
            cursor: pointer;
            font-weight: bold;
            transition: all 0.3s;
        }
        button:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.2);
        }
        button:active {
            transform: translateY(0);
        }
        .crash-list {
            max-height: 400px;
            overflow-y: auto;
        }
        .crash-item {
            background: #f9fafb;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 10px;
            border-left: 4px solid #ef4444;
        }
        .crash-item.with-note {
            border-left-color: #3b82f6;
        }
        .crash-time {
            font-weight: bold;
            color: #ef4444;
        }
        .crash-data {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 10px;
            margin: 10px 0;
            font-size: 0.9em;
        }
        .crash-note {
            background: #fff;
            padding: 10px;
            border-radius: 5px;
            margin-top: 10px;
            font-style: italic;
            color: #666;
        }
        .note-input {
            width: 100%;
            margin-top: 10px;
        }
        @media (max-width: 768px) {
            .chart-container {
                height: 250px;
            }
            .controls {
                flex-direction: column;
            }
            input, button, select {
                width: 100%;
            }
        }
        .connection-status {
            position: fixed;
            top: 10px;
            right: 10px;
            padding: 8px 15px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: bold;
            z-index: 1000;
        }
        .connected {
            background: #22c55e;
            color: white;
        }
        .disconnected {
            background: #ef4444;
            color: white;
        }
    </style>
</head>
<body>
    <div id="connectionStatus" class="connection-status disconnected">● DISCONNECTED</div>
    
    <div class="container">
        <div class="header">
            <h1>🛡️ Smart Helmet Dashboard</h1>
            <p>Real-Time Crash Detection & Monitoring System</p>
            <div class="status">
                <div class="status-item">GPS: <span id="gpsStatus" class="status-warn">Searching...</span></div>
                <div class="status-item">Battery: <span id="batteryStatus">--</span>%</div>
                <div class="status-item">Crashes: <span id="crashCount">0</span></div>
                <div class="status-item">Status: <span id="systemStatus" class="status-ok">Monitoring</span></div>
            </div>
        </div>

        <div class="card">
            <h2>⚙️ Configuration</h2>
            <div class="controls">
                <input type="number" id="accThreshold" placeholder="Acc Threshold (g)" step="0.1" min="1" max="10">
                <input type="number" id="gyroThreshold" placeholder="Gyro Threshold (°/s)" step="10" min="50" max="300">
                <input type="number" id="impactTime" placeholder="Impact Time (ms)" step="5" min="10" max="100">
                <button onclick="updateThresholds()">Update Thresholds</button>
                <button onclick="getCurrentSettings()">Get Current Settings</button>
            </div>
        </div>

        <div class="card">
            <h2>📊 Live Acceleration</h2>
            <div class="chart-container">
                <canvas id="accChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>📊 Live Gyroscope</h2>
            <div class="chart-container">
                <canvas id="gyroChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>🚨 Crash History</h2>
            <div class="controls">
                <button onclick="refreshCrashHistory()">Refresh History</button>
                <button onclick="downloadCrashHistory()">Download CSV</button>
                <button onclick="clearAllCrashes()">Clear All</button>
            </div>
            <div id="crashList" class="crash-list">
                <p style="text-align: center; color: #999; padding: 20px;">No crashes recorded</p>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let accChart, gyroChart;
        const maxDataPoints = 50;
        let accData = [];
        let gyroData = [];
        let timeLabels = [];
        let crashMarkers = [];

        // Initialize WebSocket
        function initWebSocket() {
            const wsUrl = 'ws://' + window.location.hostname + ':81';
            ws = new WebSocket(wsUrl);
            
            ws.onopen = function() {
                console.log('WebSocket Connected');
                document.getElementById('connectionStatus').className = 'connection-status connected';
                document.getElementById('connectionStatus').textContent = '● CONNECTED';
            };
            
            ws.onclose = function() {
                console.log('WebSocket Disconnected');
                document.getElementById('connectionStatus').className = 'connection-status disconnected';
                document.getElementById('connectionStatus').textContent = '● DISCONNECTED';
                setTimeout(initWebSocket, 3000);
            };
            
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    handleWebSocketData(data);
                } catch(e) {
                    console.error('Error parsing data:', e);
                }
            };
        }

        // Handle incoming WebSocket data
        function handleWebSocketData(data) {
            if (data.type === 'sensor') {
                updateSensorData(data);
            } else if (data.type === 'status') {
                updateStatus(data);
            } else if (data.type === 'settings') {
                displaySettings(data);
            }
        }

        // Update sensor data and charts
        function updateSensorData(data) {
            accData.push(data.acc);
            gyroData.push(data.gyro);
            timeLabels.push(data.time);
            crashMarkers.push(data.crash || false);
            
            if (accData.length > maxDataPoints) {
                accData.shift();
                gyroData.shift();
                timeLabels.shift();
                crashMarkers.shift();
            }
            
            updateCharts();
        }

        // Update status display
        function updateStatus(data) {
            document.getElementById('gpsStatus').textContent = data.gps ? 'Connected' : 'Searching...';
            document.getElementById('gpsStatus').className = data.gps ? 'status-ok' : 'status-warn';
            document.getElementById('batteryStatus').textContent = data.battery;
            document.getElementById('crashCount').textContent = data.crashes;
            document.getElementById('systemStatus').textContent = data.status;
            document.getElementById('systemStatus').className = data.status === 'CRASH!' ? 'status-danger' : 'status-ok';
        }

        // Initialize charts
        function initCharts() {
            const accCtx = document.getElementById('accChart').getContext('2d');
            const gyroCtx = document.getElementById('gyroChart').getContext('2d');

            const chartOptions = {
                responsive: true,
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    y: { beginAtZero: true },
                    x: { display: false }
                },
                plugins: {
                    legend: { display: false }
                }
            };

            accChart = new Chart(accCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Acceleration (g)',
                        data: [],
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        borderWidth: 2,
                        pointRadius: 0,
                        tension: 0.4
                    }]
                },
                options: chartOptions
            });

            gyroChart = new Chart(gyroCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Gyroscope (°/s)',
                        data: [],
                        borderColor: '#8b5cf6',
                        backgroundColor: 'rgba(139, 92, 246, 0.1)',
                        borderWidth: 2,
                        pointRadius: 0,
                        tension: 0.4
                    }]
                },
                options: chartOptions
            });
        }

        // Update charts with new data
        function updateCharts() {
            // Update acceleration chart
            accChart.data.labels = timeLabels;
            accChart.data.datasets[0].data = accData;
            
            // Highlight crash points in red
            accChart.data.datasets[0].pointBackgroundColor = crashMarkers.map(c => c ? '#ef4444' : '#3b82f6');
            accChart.data.datasets[0].pointRadius = crashMarkers.map(c => c ? 6 : 0);
            accChart.data.datasets[0].borderColor = crashMarkers.map((c, i) => c ? '#ef4444' : '#3b82f6');
            
            accChart.update('none');

            // Update gyroscope chart
            gyroChart.data.labels = timeLabels;
            gyroChart.data.datasets[0].data = gyroData;
            
            gyroChart.data.datasets[0].pointBackgroundColor = crashMarkers.map(c => c ? '#ef4444' : '#8b5cf6');
            gyroChart.data.datasets[0].pointRadius = crashMarkers.map(c => c ? 6 : 0);
            gyroChart.data.datasets[0].borderColor = crashMarkers.map((c, i) => c ? '#ef4444' : '#8b5cf6');
            
            gyroChart.update('none');
        }

        // Update thresholds
        function updateThresholds() {
            const acc = document.getElementById('accThreshold').value;
            const gyro = document.getElementById('gyroThreshold').value;
            const impact = document.getElementById('impactTime').value;
            
            fetch('/api/settings', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({
                    accThreshold: parseFloat(acc) || 2.5,
                    gyroThreshold: parseFloat(gyro) || 120.0,
                    impactTime: parseInt(impact) || 30
                })
            })
            .then(response => response.json())
            .then(data => {
                alert('Settings updated successfully!');
                getCurrentSettings();
            })
            .catch(error => alert('Error updating settings'));
        }

        // Get current settings
        function getCurrentSettings() {
            fetch('/api/settings')
                .then(response => response.json())
                .then(data => displaySettings(data))
                .catch(error => console.error('Error getting settings'));
        }

        // Display settings
        function displaySettings(data) {
            document.getElementById('accThreshold').value = data.accThreshold;
            document.getElementById('gyroThreshold').value = data.gyroThreshold;
            document.getElementById('impactTime').value = data.impactTime;
            alert('Current Settings:\n' +
                  'Acceleration: ' + data.accThreshold + 'g\n' +
                  'Gyroscope: ' + data.gyroThreshold + '°/s\n' +
                  'Impact Time: ' + data.impactTime + 'ms');
        }

        // Refresh crash history
        function refreshCrashHistory() {
            fetch('/api/crashes')
                .then(response => response.json())
                .then(data => displayCrashHistory(data))
                .catch(error => console.error('Error loading crashes'));
        }

        // Display crash history
        function displayCrashHistory(crashes) {
            const listDiv = document.getElementById('crashList');
            if (crashes.length === 0) {
                listDiv.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">No crashes recorded</p>';
                return;
            }
            
            listDiv.innerHTML = crashes.map((crash, index) => `
                <div class="crash-item ${crash.note ? 'with-note' : ''}">
                    <div class="crash-time">🚨 Crash #${index + 1} - ${new Date(crash.timestamp).toLocaleString()}</div>
                    <div class="crash-data">
                        <div>📍 Lat: ${crash.latitude.toFixed(6)}</div>
                        <div>📍 Lon: ${crash.longitude.toFixed(6)}</div>
                        <div>⚡ Acc: ${crash.accMag.toFixed(2)}g</div>
                        <div>🔄 Gyro: ${crash.gyroMag.toFixed(1)}°/s</div>
                        <div>🔋 Battery: ${crash.battery}%</div>
                    </div>
                    ${crash.note ? `<div class="crash-note">📝 ${crash.note}</div>` : ''}
                    <input type="text" class="note-input" id="note${index}" placeholder="Add note..." value="${crash.note || ''}">
                    <button onclick="saveNote(${index})">Save Note</button>
                    <a href="https://maps.google.com/?q=${crash.latitude},${crash.longitude}" target="_blank">
                        <button>View on Map</button>
                    </a>
                </div>
            `).join('');
        }

        // Save crash note
        function saveNote(index) {
            const note = document.getElementById('note' + index).value;
            fetch('/api/crash/note', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ index: index, note: note })
            })
            .then(response => response.json())
            .then(data => {
                alert('Note saved!');
                refreshCrashHistory();
            })
            .catch(error => alert('Error saving note'));
        }

        // Download crash history
        function downloadCrashHistory() {
            window.location.href = '/api/download';
        }

        // Clear all crashes
        function clearAllCrashes() {
            if (confirm('Are you sure you want to clear all crash records?')) {
                fetch('/api/clear', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        alert('All crashes cleared!');
                        refreshCrashHistory();
                    })
                    .catch(error => alert('Error clearing crashes'));
            }
        }

        // Initialize on page load
        window.onload = function() {
            initCharts();
            initWebSocket();
            refreshCrashHistory();
            getCurrentSettings();
        };
    </script>
</body>
</html>
)rawliteral";

/* ================= UTILITY FUNCTIONS ================= */

String readSIMResponse(unsigned long timeout = 3000) {
  String response = "";
  unsigned long start = millis();
  while (millis() - start < timeout) {
    while (sim800.available()) {
      response += char(sim800.read());
    }
  }
  return response;
}

bool sim800Ready() {
  sim800.println("AT");
  delay(100);
  if (readSIMResponse(1000).indexOf("OK") == -1) return false;
  
  sim800.println("AT+CREG?");
  delay(100);
  String r = readSIMResponse(2000);
  if (r.indexOf(",1") == -1 && r.indexOf(",5") == -1) return false;
  
  return true;
}

/* ================= BUZZER FUNCTIONS ================= */

void playPowerOnTone() {
  tone(BUZZER_PIN, NOTE_C5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_E5, 200);
  delay(250);
  tone(BUZZER_PIN, NOTE_G5, 300);
  delay(350);
  noTone(BUZZER_PIN);
}

void startWarningTone() {
  warningTonePlaying = true;
  warningToneStart = millis();
  tone(BUZZER_PIN, NOTE_A5);
  Serial.println("🚨 Warning tone started");
}

void stopWarningToneStartBeep() {
  warningTonePlaying = false;
  noTone(BUZZER_PIN);
  beepTonePlaying = true;
  beepToneStart = millis();
  Serial.println("🔊 Beep pattern started");
}

void handleBeepPattern() {
  if (!beepTonePlaying) return;
  
  unsigned long elapsed = millis() - beepToneStart;
  if (elapsed >= 3000) {
    beepTonePlaying = false;
    noTone(BUZZER_PIN);
    Serial.println("🔇 Beep pattern completed");
    return;
  }
  
  unsigned long cycleTime = elapsed % 400;
  if (cycleTime < 200) {
    tone(BUZZER_PIN, NOTE_E5);
  } else {
    noTone(BUZZER_PIN);
  }
}

void stopAllBuzzerSounds() {
  warningTonePlaying = false;
  beepTonePlaying = false;
  noTone(BUZZER_PIN);
}

/* ================= MPU FUNCTIONS ================= */

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  Serial.println("🟡 Calibrating gyro...");
  delay(2000);

  long sx=0, sy=0, sz=0;
  for(int i=0; i<500; i++){
    readMPU();
    sx += GyX;
    sy += GyY;
    sz += GyZ;
    delay(5);
  }

  gyroBiasX = sx / 500.0;
  gyroBiasY = sy / 500.0;
  gyroBiasZ = sz / 500.0;

  Serial.println("✅ Gyro calibrated");
}

/* ================= BATTERY MONITORING ================= */

int readBatteryPercent() {
  int adcValue = analogRead(BATTERY_PIN);
  float voltage = (adcValue / 4095.0) * 3.3;
  float batteryVoltage = voltage * VOLTAGE_DIVIDER_RATIO;
  int percent = (int)((batteryVoltage - BATTERY_EMPTY_VOLTAGE) / 
                      (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE) * 100.0);
  return constrain(percent, 0, 100);
}

/* ================= SMS FUNCTIONS ================= */

bool sendSMSToNumber(float lat, float lon, const char* phoneNumber) {
  String msg = "🚨 ACCIDENT DETECTED!\n";
  msg += "Emergency alert from Smart Helmet.\n\n";
  msg += "Location Details:\n";
  msg += "Latitude: " + String(lat, 6) + "\n";
  msg += "Longitude: " + String(lon, 6) + "\n\n";
  msg += "View on Google Maps:\n";
  msg += "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lon, 6);

  Serial.print("📤 Sending SMS to: ");
  Serial.println(phoneNumber);

  sim800.println("AT+CMGF=1");
  delay(100);
  if (readSIMResponse(1000).indexOf("OK") == -1) return false;

  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(500);

  if (readSIMResponse(2000).indexOf(">") == -1) return false;

  sim800.print(msg);
  delay(100);
  sim800.write(26);

  String result = readSIMResponse(10000);
  return (result.indexOf("+CMGS:") != -1 || result.indexOf("OK") != -1);
}

bool sendSMSToAllContacts(float lat, float lon) {
  if (!sim800Ready()) return false;

  int successCount = 0;
  if (sendSMSToNumber(lat, lon, PHONE_NUMBER_1)) successCount++;
  delay(2000);
  if (sendSMSToNumber(lat, lon, PHONE_NUMBER_2)) successCount++;
  delay(2000);
  if (sendSMSToNumber(lat, lon, PHONE_NUMBER_3)) successCount++;
  
  Serial.print("✅ SMS sent to ");
  Serial.print(successCount);
  Serial.println(" contacts");
  
  return successCount > 0;
}

/* ================= CRASH DATA LOGGING ================= */

void initCrashLogging() {
  Serial.println("🔧 Initializing crash logging...");
  if (!SPIFFS.begin(true)) {
    Serial.println("❌ SPIFFS mount failed");
    return;
  }
  
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    file = SPIFFS.open("/crashes.csv", FILE_WRITE);
    if (file) {
      file.println("Timestamp,Latitude,Longitude,AccMag,GyroMag,Battery%,Note");
      file.close();
    }
  } else {
    file.close();
  }
  Serial.println("✅ Crash logging initialized");
}

void logCrashData(float lat, float lon, float accMag, float gyroMag, int battery) {
  Serial.println("💾 Logging crash data...");
  
  File file = SPIFFS.open("/crashes.csv", FILE_APPEND);
  if (!file) {
    Serial.println("❌ Failed to open crash log");
    return;
  }
  
  file.print(millis());
  file.print(",");
  file.print(lat, 6);
  file.print(",");
  file.print(lon, 6);
  file.print(",");
  file.print(accMag, 2);
  file.print(",");
  file.print(gyroMag, 2);
  file.print(",");
  file.print(battery);
  file.print(",");
  file.println("");  // Empty note initially
  
  file.close();
  crashLogCount++;
  
  Serial.println("✅ Crash data logged");
}

/* ================= WEB SERVER HANDLERS ================= */

void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

void handleGetSettings() {
  StaticJsonDocument<200> doc;
  doc["accThreshold"] = ACC_THRESHOLD;
  doc["gyroThreshold"] = GYRO_THRESHOLD;
  doc["impactTime"] = IMPACT_TIME_MS;
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleUpdateSettings() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    ACC_THRESHOLD = doc["accThreshold"] | ACC_THRESHOLD;
    GYRO_THRESHOLD = doc["gyroThreshold"] | GYRO_THRESHOLD;
    IMPACT_TIME_MS = doc["impactTime"] | IMPACT_TIME_MS;
    
    Serial.println("✅ Thresholds updated via web");
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void handleGetCrashes() {
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    server.send(200, "application/json", "[]");
    return;
  }
  
  String json = "[";
  file.readStringUntil('\n'); // Skip header
  
  bool first = true;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() < 5) continue;
    
    int idx = 0;
    String parts[7];
    int partIdx = 0;
    
    for (int i = 0; i < line.length() && partIdx < 7; i++) {
      if (line[i] == ',') {
        partIdx++;
      } else {
        parts[partIdx] += line[i];
      }
    }
    
    if (!first) json += ",";
    first = false;
    
    json += "{";
    json += "\"timestamp\":" + parts[0] + ",";
    json += "\"latitude\":" + parts[1] + ",";
    json += "\"longitude\":" + parts[2] + ",";
    json += "\"accMag\":" + parts[3] + ",";
    json += "\"gyroMag\":" + parts[4] + ",";
    json += "\"battery\":" + parts[5] + ",";
    json += "\"note\":\"" + parts[6] + "\"";
    json += "}";
  }
  
  json += "]";
  file.close();
  
  server.send(200, "application/json", json);
}

void handleSaveNote() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"status\":\"error\"}");
    return;
  }
  
  StaticJsonDocument<200> doc;
  deserializeJson(doc, server.arg("plain"));
  
  int index = doc["index"];
  String note = doc["note"].as<String>();
  
  // Read all crashes
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    server.send(500, "application/json", "{\"status\":\"error\"}");
    return;
  }
  
  String allData = "";
  String header = file.readStringUntil('\n');
  allData += header + "\n";
  
  int currentIndex = 0;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() < 5) continue;
    
    if (currentIndex == index) {
      // Replace note
      int lastComma = line.lastIndexOf(',');
      if (lastComma > 0) {
        line = line.substring(0, lastComma + 1) + note;
      }
    }
    
    allData += line + "\n";
    currentIndex++;
  }
  file.close();
  
  // Write back
  file = SPIFFS.open("/crashes.csv", FILE_WRITE);
  if (file) {
    file.print(allData);
    file.close();
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(500, "application/json", "{\"status\":\"error\"}");
  }
}

void handleDownload() {
  File file = SPIFFS.open("/crashes.csv", FILE_READ);
  if (!file) {
    server.send(404, "text/plain", "No crash data found");
    return;
  }
  
  server.streamFile(file, "text/csv");
  file.close();
}

void handleClear() {
  SPIFFS.remove("/crashes.csv");
  initCrashLogging();
  crashLogCount = 0;
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

/* ================= WEBSOCKET HANDLER ================= */

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // Handle WebSocket events if needed
}

void sendSensorDataViaWebSocket(float accMag, float gyroMag, bool isCrash) {
  StaticJsonDocument<200> doc;
  doc["type"] = "sensor";
  doc["acc"] = accMag;
  doc["gyro"] = gyroMag;
  doc["time"] = millis();
  doc["crash"] = isCrash;
  
  String json;
  serializeJson(doc, json);
  webSocket.broadcastTXT(json);
}

void sendStatusViaWebSocket() {
  StaticJsonDocument<200> doc;
  doc["type"] = "status";
  doc["gps"] = gpsConnected;
  doc["battery"] = batteryPercent;
  doc["crashes"] = crashLogCount;
  doc["status"] = crashDetected ? "CRASH!" : "Monitoring";
  
  String json;
  serializeJson(doc, json);
  webSocket.broadcastTXT(json);
}

/* ================= OLED DISPLAY FUNCTIONS ================= */

void updateOLEDDisplay() {
  if (!displayAvailable) return;
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Smart Helmet v3.0");
  display.println("Web Dashboard");
  display.println("----------------");
  display.print("IP: ");
  display.println(webDashboardIP);
  display.print("GPS: ");
  display.println(gpsConnected ? "OK" : "...");
  display.print("Battery: ");
  display.print(batteryPercent);
  display.println("%");
  display.print("Crashes: ");
  display.println(crashLogCount);
  display.display();
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n========================================");
  Serial.println("  SMART HELMET WEB DASHBOARD v3.0  ");
  Serial.println("========================================\n");

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(CANCEL_BTN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);
  noTone(BUZZER_PIN);

  // Initialize I2C and MPU6050
  Serial.println("🔧 Initializing MPU6050...");
  Wire.begin(21, 22);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  delay(100);
  Serial.println("✅ MPU6050 initialized");

  calibrateGyro();

  // Initialize OLED
  Serial.println("🔧 Initializing OLED...");
  if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    displayAvailable = true;
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Smart Helmet");
    display.println("Connecting WiFi...");
    display.display();
    Serial.println("✅ OLED initialized");
  } else {
    displayAvailable = false;
    Serial.println("⚠️  OLED disabled");
  }

  // Initialize GPS
  Serial.println("🔧 Initializing GPS...");
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("✅ GPS initialized");

  // Initialize SIM800L
  Serial.println("🔧 Initializing SIM800L...");
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(3000);
  
  if (sim800Ready()) {
    Serial.println("✅ SIM800L ready");
    playPowerOnTone();
  } else {
    Serial.println("⚠️  SIM800L not ready");
  }

  // Initialize SPIFFS
  initCrashLogging();

  // Connect to WiFi
  Serial.println("🔧 Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    Serial.print(".");
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
    Serial.print("📱 Dashboard URL: http://");
    Serial.println(WiFi.localIP());
    webDashboardIP = WiFi.localIP().toString();
    
    // Setup web server
    server.on("/", handleRoot);
    server.on("/api/settings", HTTP_GET, handleGetSettings);
    server.on("/api/settings", HTTP_POST, handleUpdateSettings);
    server.on("/api/crashes", handleGetCrashes);
    server.on("/api/crash/note", HTTP_POST, handleSaveNote);
    server.on("/api/download", handleDownload);
    server.on("/api/clear", HTTP_POST, handleClear);
    server.begin();
    Serial.println("✅ Web server started");
    
    // Setup WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("✅ WebSocket started");
    
  } else {
    Serial.println("\n❌ WiFi connection failed");
    Serial.println("⚠️  Running without web dashboard");
  }

  updateOLEDDisplay();

  Serial.println("\n========================================");
  Serial.println("   SYSTEM READY - Monitoring...   ");
  Serial.println("========================================\n");
}

/* ================= LOOP ================= */

void loop() {
  // Handle web server
  server.handleClient();
  webSocket.loop();

  // Cancel button
  if (digitalRead(CANCEL_BTN) == LOW) {
    crashDetected = false;
    smsSent = false;
    stopAllBuzzerSounds();
    Serial.println("❌ ALERT CANCELLED");
    delay(1000);
  }

  // Buzzer patterns
  if (warningTonePlaying && millis() - warningToneStart >= 10000) {
    stopWarningToneStartBeep();
  }
  
  if (beepTonePlaying) {
    handleBeepPattern();
  }

  // Read sensor data
  readMPU();

  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  float gx = (GyX - gyroBiasX) / 131.0;
  float gy = (GyY - gyroBiasY) / 131.0;
  float gz = (GyZ - gyroBiasZ) / 131.0;

  float accMag = sqrt(ax*ax + ay*ay + az*az);
  float gyroMag = sqrt(gx*gx + gy*gy + gz*gz);

  // Send data via WebSocket
  if (millis() - lastDataUpdate >= 200) {
    sendSensorDataViaWebSocket(accMag, gyroMag, crashDetected);
    lastDataUpdate = millis();
  }

  Serial.print("ACC: "); Serial.print(accMag, 2);
  Serial.print(" | GYRO: "); Serial.print(gyroMag, 1);

  // Crash detection
  if (accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    if (impactStart == 0) {
      impactStart = millis();
      Serial.print(" [IMPACT]");
    }
    
    if (millis() - impactStart > IMPACT_TIME_MS) {
      if (!crashDetected) {
        crashDetected = true;
        crashTime = millis();
        startWarningTone();
        Serial.println("\n🚨🚨🚨 CRASH DETECTED 🚨🚨🚨");
      }
    }
  } else {
    impactStart = 0;
  }
  
  Serial.println();

  // GPS processing
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }
  
  gpsConnected = gps.location.isValid();

  // Emergency SMS
  if (crashDetected && gps.location.isValid() && !smsSent) {
    Serial.println("📍 GPS fix - sending SMS...");
    
    if (warningTonePlaying) {
      stopWarningToneStartBeep();
    }
    
    logCrashData(gps.location.lat(), gps.location.lng(), accMag, gyroMag, batteryPercent);
    
    if (sendSMSToAllContacts(gps.location.lat(), gps.location.lng())) {
      smsSent = true;
    }
  }

  if (crashDetected && !gps.location.isValid() && !smsSent) {
    Serial.print("⏳ Waiting GPS... Sats: ");
    Serial.println(gps.satellites.value());
  }

  if (gps.location.isValid()) {
    Serial.print("📍 GPS: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
  }
  
  // Read battery
  static unsigned long lastBatteryRead = 0;
  if (millis() - lastBatteryRead >= 10000) {
    batteryPercent = readBatteryPercent();
    lastBatteryRead = millis();
  }
  
  // Send status update
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate >= 2000) {
    sendStatusViaWebSocket();
    updateOLEDDisplay();
    lastStatusUpdate = millis();
  }

  delay(200);
}
