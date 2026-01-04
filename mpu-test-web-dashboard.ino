/* 
 * =====================================================
 * MPU TEST AND RECORDING - WEB DASHBOARD
 * =====================================================
 * 
 * Purpose: Test and record MPU6050 crash detection logic
 * 
 * Features:
 * - Real-time MPU6050 data reading (accelerometer + gyroscope)
 * - Crash detection logic testing
 * - Web dashboard for live visualization
 * - Crashes marked in RED with "Crash detected" note
 * - Download recorded data as CSV on phone/computer
 * - No GPS, no SMS, no emergency contacts - TESTING ONLY
 * 
 * HARDWARE REQUIRED:
 * ------------------
 * 1. ESP32 Development Board
 * 2. MPU6050 - 6-axis Gyroscope & Accelerometer
 * 
 * WIRING:
 * -------
 * MPU6050:
 *   - VCC  -> 3.3V
 *   - GND  -> GND
 *   - SDA  -> GPIO 21
 *   - SCL  -> GPIO 22
 * 
 * USAGE:
 * ------
 * 1. Change WiFi credentials below
 * 2. Upload to ESP32
 * 3. Open Serial Monitor to see IP address
 * 4. Open browser to http://[ESP32-IP]
 * 5. View real-time MPU data and crashes
 * 6. Download CSV data anytime
 * 
 * Author: Smart Helmet MPU Test
 * Version: 1.0
 */

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

/* ================= WIFI CONFIGURATION ================= */
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ CHANGE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ CHANGE THIS

/* ================= MPU6050 ================= */
#define MPU_ADDR 0x68
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

/* ================= THRESHOLDS (CONFIGURABLE VIA WEB) ================= */
float ACC_THRESHOLD = 2.5;      // g-force
float GYRO_THRESHOLD = 120.0;   // degrees per second
int IMPACT_TIME_MS = 30;        // milliseconds

/* ================= STATE VARIABLES ================= */
bool crashDetected = false;
unsigned long impactStart = 0;
int crashCount = 0;

/* ================= WEB SERVER & WEBSOCKET ================= */
WebServer server(80);
WebSocketsServer webSocket(81);

/* ================= DATA STORAGE ================= */
#define MAX_DATA_POINTS 1000
struct DataPoint {
  unsigned long timestamp;
  float accMag;
  float gyroMag;
  bool isCrash;
};
DataPoint dataHistory[MAX_DATA_POINTS];
int dataIndex = 0;

/* ================= MPU FUNCTIONS ================= */

void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  
  Wire.read(); Wire.read(); // Temperature
  
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

void calibrateGyro() {
  Serial.println("🟡 Calibrating gyro (keep still for 2 seconds)...");
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
  Serial.print("Bias X: "); Serial.print(gyroBiasX);
  Serial.print(" Y: "); Serial.print(gyroBiasY);
  Serial.print(" Z: "); Serial.println(gyroBiasZ);
}

/* ================= DATA STORAGE FUNCTIONS ================= */

void storeDataPoint(float accMag, float gyroMag, bool isCrash) {
  dataHistory[dataIndex].timestamp = millis();
  dataHistory[dataIndex].accMag = accMag;
  dataHistory[dataIndex].gyroMag = gyroMag;
  dataHistory[dataIndex].isCrash = isCrash;
  
  dataIndex = (dataIndex + 1) % MAX_DATA_POINTS;
}

/* ================= WEB SERVER HANDLERS ================= */

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MPU Test Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 30px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
        }
        .subtitle {
            color: #666;
            font-size: 14px;
        }
        .status {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            gap: 15px;
            margin-top: 20px;
        }
        .status-item {
            background: #f9fafb;
            padding: 10px 20px;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
        }
        .status-ok { color: #22c55e; }
        .status-warn { color: #f59e0b; }
        .status-error { color: #ef4444; }
        .card {
            background: white;
            padding: 25px;
            border-radius: 15px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }
        h2 {
            color: #333;
            margin-bottom: 20px;
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
        input, button {
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
            background: #fee;
            padding: 15px;
            margin-bottom: 10px;
            border-radius: 10px;
            border-left: 4px solid #ef4444;
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
        @media (max-width: 768px) {
            .chart-container {
                height: 250px;
            }
            .controls {
                flex-direction: column;
            }
            input, button {
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
        .realtime-values {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 15px;
            margin-top: 20px;
        }
        .value-box {
            background: #f9fafb;
            padding: 15px;
            border-radius: 10px;
            text-align: center;
        }
        .value-label {
            font-size: 12px;
            color: #666;
            margin-bottom: 5px;
        }
        .value-number {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
        }
    </style>
</head>
<body>
    <div id="connectionStatus" class="connection-status disconnected">● DISCONNECTED</div>
    
    <div class="container">
        <div class="header">
            <h1>🧪 MPU Test Dashboard</h1>
            <p class="subtitle">Real-Time Crash Detection Testing & Data Recording</p>
            <div class="status">
                <div class="status-item">Status: <span id="systemStatus" class="status-ok">Monitoring</span></div>
                <div class="status-item">Crashes Detected: <span id="crashCount" class="status-warn">0</span></div>
                <div class="status-item">Data Points: <span id="dataPoints">0</span></div>
            </div>
        </div>

        <div class="card">
            <h2>📊 Real-Time Values</h2>
            <div class="realtime-values">
                <div class="value-box">
                    <div class="value-label">Acceleration (g)</div>
                    <div class="value-number" id="currentAcc">0.00</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Gyroscope (°/s)</div>
                    <div class="value-number" id="currentGyro">0.00</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Acc Threshold</div>
                    <div class="value-number" id="currentAccThreshold" style="font-size: 20px;">2.50</div>
                </div>
                <div class="value-box">
                    <div class="value-label">Gyro Threshold</div>
                    <div class="value-number" id="currentGyroThreshold" style="font-size: 20px;">120.0</div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>⚙️ Crash Detection Configuration</h2>
            <div class="controls">
                <input type="number" id="accThreshold" placeholder="Acc Threshold (g)" step="0.1" min="1" max="10" value="2.5">
                <input type="number" id="gyroThreshold" placeholder="Gyro Threshold (°/s)" step="10" min="50" max="300" value="120">
                <input type="number" id="impactTime" placeholder="Impact Time (ms)" step="5" min="10" max="100" value="30">
                <button onclick="updateThresholds()">Update Thresholds</button>
                <button onclick="getCurrentSettings()">Get Current Settings</button>
            </div>
        </div>

        <div class="card">
            <h2>📈 Live Acceleration Graph</h2>
            <div class="chart-container">
                <canvas id="accChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>📈 Live Gyroscope Graph</h2>
            <div class="chart-container">
                <canvas id="gyroChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2>🚨 Crash History</h2>
            <div class="controls">
                <button onclick="refreshCrashHistory()">Refresh History</button>
                <button onclick="downloadCSV()">Download CSV</button>
                <button onclick="clearAllData()">Clear All Data</button>
            </div>
            <div id="crashList" class="crash-list">
                <p style="text-align: center; color: #999; padding: 20px;">No crashes detected yet</p>
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
        let crashHistory = [];

        // Initialize charts
        function initCharts() {
            const accCtx = document.getElementById('accChart').getContext('2d');
            accChart = new Chart(accCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Acceleration (g)',
                        data: accData,
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        pointRadius: 0,
                        pointHoverRadius: 5
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Acceleration (g)'
                            }
                        },
                        x: {
                            display: false
                        }
                    },
                    plugins: {
                        legend: {
                            display: true
                        }
                    },
                    animation: {
                        duration: 0
                    }
                }
            });

            const gyroCtx = document.getElementById('gyroChart').getContext('2d');
            gyroChart = new Chart(gyroCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Gyroscope (°/s)',
                        data: gyroData,
                        borderColor: '#10b981',
                        backgroundColor: 'rgba(16, 185, 129, 0.1)',
                        borderWidth: 2,
                        tension: 0.4,
                        pointRadius: 0,
                        pointHoverRadius: 5
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Gyroscope (°/s)'
                            }
                        },
                        x: {
                            display: false
                        }
                    },
                    plugins: {
                        legend: {
                            display: true
                        }
                    },
                    animation: {
                        duration: 0
                    }
                }
            });
        }

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
            } else if (data.type === 'crash') {
                handleCrashEvent(data);
            } else if (data.type === 'settings') {
                displaySettings(data);
            }
        }

        // Update sensor data on charts
        function updateSensorData(data) {
            // Update real-time values
            document.getElementById('currentAcc').textContent = data.acc.toFixed(2);
            document.getElementById('currentGyro').textContent = data.gyro.toFixed(2);
            
            // Update charts
            if (accData.length >= maxDataPoints) {
                accData.shift();
                gyroData.shift();
                timeLabels.shift();
            }
            
            accData.push(data.acc);
            gyroData.push(data.gyro);
            timeLabels.push('');
            
            // Highlight crash points in RED
            if (data.crash) {
                // Mark crash point in red
                const lastIndex = accData.length - 1;
                accChart.data.datasets[0].pointBackgroundColor = accChart.data.datasets[0].pointBackgroundColor || [];
                accChart.data.datasets[0].pointBackgroundColor[lastIndex] = '#ef4444';
                accChart.data.datasets[0].pointRadius = accChart.data.datasets[0].pointRadius || [];
                accChart.data.datasets[0].pointRadius[lastIndex] = 8;
                
                gyroChart.data.datasets[0].pointBackgroundColor = gyroChart.data.datasets[0].pointBackgroundColor || [];
                gyroChart.data.datasets[0].pointBackgroundColor[lastIndex] = '#ef4444';
                gyroChart.data.datasets[0].pointRadius = gyroChart.data.datasets[0].pointRadius || [];
                gyroChart.data.datasets[0].pointRadius[lastIndex] = 8;
                
                document.getElementById('systemStatus').className = 'status-error';
                document.getElementById('systemStatus').textContent = 'CRASH DETECTED!';
                
                // Reset status after 3 seconds
                setTimeout(() => {
                    document.getElementById('systemStatus').className = 'status-ok';
                    document.getElementById('systemStatus').textContent = 'Monitoring';
                }, 3000);
            } else {
                const lastIndex = accData.length - 1;
                if (accChart.data.datasets[0].pointBackgroundColor) {
                    accChart.data.datasets[0].pointBackgroundColor[lastIndex] = '#3b82f6';
                    accChart.data.datasets[0].pointRadius[lastIndex] = 0;
                }
                if (gyroChart.data.datasets[0].pointBackgroundColor) {
                    gyroChart.data.datasets[0].pointBackgroundColor[lastIndex] = '#10b981';
                    gyroChart.data.datasets[0].pointRadius[lastIndex] = 0;
                }
            }
            
            accChart.update();
            gyroChart.update();
        }

        // Handle crash event
        function handleCrashEvent(data) {
            document.getElementById('crashCount').textContent = data.count;
            crashHistory.push(data);
            refreshCrashHistory();
        }

        // Update threshold configuration
        function updateThresholds() {
            const acc = document.getElementById('accThreshold').value;
            const gyro = document.getElementById('gyroThreshold').value;
            const impact = document.getElementById('impactTime').value;
            
            fetch('/updateThresholds', {
                method: 'POST',
                headers: {'Content-Type': 'application/json'},
                body: JSON.stringify({
                    acc: parseFloat(acc),
                    gyro: parseFloat(gyro),
                    impact: parseInt(impact)
                })
            })
            .then(response => response.json())
            .then(data => {
                alert('Thresholds updated successfully!');
                getCurrentSettings();
            })
            .catch(error => {
                alert('Error updating thresholds: ' + error);
            });
        }

        // Get current settings
        function getCurrentSettings() {
            fetch('/getSettings')
            .then(response => response.json())
            .then(data => {
                displaySettings(data);
            });
        }

        // Display current settings
        function displaySettings(data) {
            document.getElementById('accThreshold').value = data.acc;
            document.getElementById('gyroThreshold').value = data.gyro;
            document.getElementById('impactTime').value = data.impact;
            document.getElementById('currentAccThreshold').textContent = data.acc.toFixed(2);
            document.getElementById('currentGyroThreshold').textContent = data.gyro.toFixed(1);
        }

        // Refresh crash history
        function refreshCrashHistory() {
            fetch('/getCrashHistory')
            .then(response => response.json())
            .then(data => {
                const crashList = document.getElementById('crashList');
                if (data.crashes && data.crashes.length > 0) {
                    crashList.innerHTML = '';
                    data.crashes.forEach(crash => {
                        const crashDiv = document.createElement('div');
                        crashDiv.className = 'crash-item';
                        crashDiv.innerHTML = `
                            <div class="crash-time">🚨 CRASH DETECTED</div>
                            <div class="crash-data">
                                <div><strong>Time:</strong> ${new Date(crash.timestamp).toLocaleString()}</div>
                                <div><strong>Acceleration:</strong> ${crash.acc.toFixed(2)} g</div>
                                <div><strong>Gyroscope:</strong> ${crash.gyro.toFixed(2)} °/s</div>
                            </div>
                        `;
                        crashList.appendChild(crashDiv);
                    });
                    document.getElementById('crashCount').textContent = data.crashes.length;
                } else {
                    crashList.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">No crashes detected yet</p>';
                }
            });
        }

        // Download CSV
        function downloadCSV() {
            window.location.href = '/downloadCSV';
        }

        // Clear all data
        function clearAllData() {
            if (confirm('Are you sure you want to clear all recorded data?')) {
                fetch('/clearData', {method: 'POST'})
                .then(response => response.json())
                .then(data => {
                    alert('All data cleared!');
                    crashHistory = [];
                    refreshCrashHistory();
                    document.getElementById('crashCount').textContent = '0';
                    document.getElementById('dataPoints').textContent = '0';
                });
            }
        }

        // Initialize on page load
        window.onload = function() {
            initCharts();
            initWebSocket();
            getCurrentSettings();
            refreshCrashHistory();
            
            // Update data points counter periodically
            setInterval(() => {
                document.getElementById('dataPoints').textContent = accData.length;
            }, 1000);
        };
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleUpdateThresholds() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    deserializeJson(doc, server.arg("plain"));
    
    ACC_THRESHOLD = doc["acc"];
    GYRO_THRESHOLD = doc["gyro"];
    IMPACT_TIME_MS = doc["impact"];
    
    Serial.println("Thresholds updated:");
    Serial.print("ACC: "); Serial.print(ACC_THRESHOLD);
    Serial.print(" GYRO: "); Serial.print(GYRO_THRESHOLD);
    Serial.print(" IMPACT: "); Serial.println(IMPACT_TIME_MS);
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\"}");
  }
}

void handleGetSettings() {
  StaticJsonDocument<200> doc;
  doc["acc"] = ACC_THRESHOLD;
  doc["gyro"] = GYRO_THRESHOLD;
  doc["impact"] = IMPACT_TIME_MS;
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleGetCrashHistory() {
  StaticJsonDocument<4096> doc;
  JsonArray crashes = doc.createNestedArray("crashes");
  
  // Find all crash data points
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].isCrash && dataHistory[i].timestamp > 0) {
      JsonObject crash = crashes.createNestedObject();
      crash["timestamp"] = dataHistory[i].timestamp;
      crash["acc"] = dataHistory[i].accMag;
      crash["gyro"] = dataHistory[i].gyroMag;
    }
  }
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleDownloadCSV() {
  String csv = "Timestamp (ms),Acceleration (g),Gyroscope (deg/s),Crash Detected\n";
  
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].timestamp > 0) {
      csv += String(dataHistory[i].timestamp) + ",";
      csv += String(dataHistory[i].accMag, 2) + ",";
      csv += String(dataHistory[i].gyroMag, 2) + ",";
      csv += dataHistory[i].isCrash ? "YES" : "NO";
      csv += "\n";
    }
  }
  
  server.send(200, "text/csv", csv);
}

void handleClearData() {
  // Clear all data
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    dataHistory[i].timestamp = 0;
    dataHistory[i].accMag = 0;
    dataHistory[i].gyroMag = 0;
    dataHistory[i].isCrash = false;
  }
  dataIndex = 0;
  crashCount = 0;
  
  Serial.println("All data cleared");
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

/* ================= WEBSOCKET HANDLER ================= */

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // Not used in this simple implementation
}

/* ================= SETUP ================= */

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("   MPU TEST & RECORDING SYSTEM         ");
  Serial.println("========================================\n");
  
  // Initialize I2C for MPU6050
  Serial.println("🔧 Initializing MPU6050...");
  Wire.begin(21, 22);
  
  // Wake up MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission();
  delay(100);
  Serial.println("✅ MPU6050 initialized");
  
  // Calibrate gyroscope
  calibrateGyro();
  
  // Initialize data storage
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    dataHistory[i].timestamp = 0;
  }
  
  // Connect to WiFi
  Serial.println("🔧 Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected!");
    Serial.print("📱 IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("🌐 Open browser to: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ WiFi connection failed!");
    Serial.println("⚠️ Check SSID and password");
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/updateThresholds", HTTP_POST, handleUpdateThresholds);
  server.on("/getSettings", handleGetSettings);
  server.on("/getCrashHistory", handleGetCrashHistory);
  server.on("/downloadCSV", handleDownloadCSV);
  server.on("/clearData", HTTP_POST, handleClearData);
  
  // Start web server
  server.begin();
  Serial.println("✅ Web server started");
  
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✅ WebSocket server started");
  
  Serial.println("\n========================================");
  Serial.println("   SYSTEM READY - TESTING MODE         ");
  Serial.println("========================================\n");
}

/* ================= LOOP ================= */

void loop() {
  // Handle web server
  server.handleClient();
  webSocket.loop();
  
  // Read MPU data
  readMPU();
  
  // Convert to physical units
  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;
  
  float gx = (GyX - gyroBiasX) / 131.0;
  float gy = (GyY - gyroBiasY) / 131.0;
  float gz = (GyZ - gyroBiasZ) / 131.0;
  
  float accMag = sqrt(ax*ax + ay*ay + az*az);
  float gyroMag = sqrt(gx*gx + gy*gy + gz*gz);
  
  // Print to serial
  Serial.print("ACC(g): "); Serial.print(accMag, 2);
  Serial.print(" | GYRO(dps): "); Serial.print(gyroMag, 1);
  
  // Crash detection logic
  bool crashNow = false;
  
  if (accMag > ACC_THRESHOLD && gyroMag > GYRO_THRESHOLD) {
    if (impactStart == 0) {
      impactStart = millis();
      Serial.print(" [IMPACT DETECTED]");
    }
    
    if (millis() - impactStart > IMPACT_TIME_MS) {
      if (!crashDetected) {
        crashDetected = true;
        crashNow = true;
        crashCount++;
        
        Serial.print("\n🚨🚨🚨 CRASH DETECTED 🚨🚨🚨 (Count: ");
        Serial.print(crashCount);
        Serial.println(")");
        
        // Send crash event via WebSocket
        StaticJsonDocument<200> crashDoc;
        crashDoc["type"] = "crash";
        crashDoc["count"] = crashCount;
        crashDoc["acc"] = accMag;
        crashDoc["gyro"] = gyroMag;
        crashDoc["timestamp"] = millis();
        
        String crashJson;
        serializeJson(crashDoc, crashJson);
        webSocket.broadcastTXT(crashJson);
        
        // Reset crash flag after 1 second
        delay(1000);
        crashDetected = false;
      }
    }
  } else {
    impactStart = 0;
  }
  
  Serial.println();
  
  // Store data point
  storeDataPoint(accMag, gyroMag, crashNow);
  
  // Send sensor data via WebSocket
  StaticJsonDocument<200> doc;
  doc["type"] = "sensor";
  doc["acc"] = accMag;
  doc["gyro"] = gyroMag;
  doc["crash"] = crashNow;
  
  String json;
  serializeJson(doc, json);
  webSocket.broadcastTXT(json);
  
  delay(200);  // Sample rate: ~5 Hz
}
