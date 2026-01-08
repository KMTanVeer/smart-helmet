/* 
 * =====================================================
 * NEW MPU TEST WITH WEB DASHBOARD - DHAKA TIMEZONE
 * =====================================================
 * 
 * Purpose: Test and record MPU6050 crash detection logic with real timezone
 * 
 * Features:
 * - Real-time MPU6050 data reading (accelerometer + gyroscope)
 * - Crash detection logic testing
 * - Web dashboard for live visualization with graphs
 * - Crashes marked in RED with "Crash detected" note
 * - Real timezone support for Dhaka, Bangladesh (GMT+6)
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
 * 5. View real-time MPU data and crashes with Dhaka timezone
 * 6. Download CSV data anytime
 * 
 * Author: Smart Helmet MPU Test
 * Version: 2.0 - With Dhaka Timezone
 */

#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/* ================= WIFI CONFIGURATION ================= */
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // ⚠️ CHANGE THIS
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // ⚠️ CHANGE THIS

/* ================= NTP CONFIGURATION FOR DHAKA TIMEZONE ================= */
// Dhaka, Bangladesh timezone: GMT+6 (21600 seconds)
const long UTC_OFFSET_SECONDS = 21600;  // 6 hours * 3600 seconds
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", UTC_OFFSET_SECONDS, 60000);

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
unsigned long crashResetTime = 0;  // Time when crash flag should be reset
int crashCount = 0;
const unsigned long CRASH_RESET_DELAY = 1000;  // Reset after 1 second
const size_t TIME_BUFFER_SIZE = 30;  // Buffer size for time string

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
  String timeString;  // Human-readable Dhaka time
};
DataPoint dataHistory[MAX_DATA_POINTS];
int dataIndex = 0;

/* ================= UTILITY FUNCTIONS ================= */

String getDhakaTime() {
  if (!timeClient.isTimeSet()) {
    return "Time not synced";
  }
  
  unsigned long epochTime = timeClient.getEpochTime();
  time_t timeValue = epochTime;
  struct tm *timeInfo = gmtime(&timeValue);
  
  char timeBuffer[TIME_BUFFER_SIZE];
  strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeInfo);
  return String(timeBuffer);
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
  dataHistory[dataIndex].timeString = getDhakaTime();
  
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
    <title>Smart Helmet IoT Analytics Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
        }
        .header {
            background: linear-gradient(135deg, rgba(255,255,255,0.95) 0%, rgba(255,255,255,0.9) 100%);
            padding: 30px;
            border-radius: 20px;
            margin-bottom: 25px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
            text-align: center;
        }
        h1 {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            font-size: 2em;
            font-weight: 700;
            margin-bottom: 8px;
        }
        .subtitle {
            color: #64748b;
            font-size: 14px;
            font-weight: 500;
        }
        .timezone-badge {
            background: linear-gradient(135deg, #10b981 0%, #059669 100%);
            color: white;
            padding: 6px 16px;
            border-radius: 20px;
            font-size: 12px;
            font-weight: 600;
            display: inline-block;
            margin-top: 12px;
            box-shadow: 0 2px 8px rgba(16, 185, 129, 0.3);
        }
        .current-time {
            font-size: 16px;
            color: #475569;
            margin-top: 12px;
            font-weight: 600;
        }
        .connection-status {
            position: fixed;
            top: 20px;
            right: 20px;
            padding: 10px 18px;
            border-radius: 25px;
            font-size: 12px;
            font-weight: 700;
            z-index: 1000;
            box-shadow: 0 4px 12px rgba(0,0,0,0.15);
        }
        .connected {
            background: linear-gradient(135deg, #10b981 0%, #059669 100%);
            color: white;
        }
        .disconnected {
            background: linear-gradient(135deg, #ef4444 0%, #dc2626 100%);
            color: white;
        }
        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 25px;
        }
        .metric-card {
            background: linear-gradient(135deg, rgba(255,255,255,0.95) 0%, rgba(255,255,255,0.9) 100%);
            padding: 24px;
            border-radius: 20px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
            transition: transform 0.3s, box-shadow 0.3s;
        }
        .metric-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 12px 40px rgba(0,0,0,0.15);
        }
        .metric-label {
            font-size: 13px;
            font-weight: 600;
            color: #64748b;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 8px;
        }
        .metric-value {
            font-size: 36px;
            font-weight: 700;
            margin-bottom: 8px;
        }
        .metric-value.normal {
            color: #10b981;
        }
        .metric-value.warning {
            color: #f59e0b;
        }
        .metric-value.danger {
            color: #ef4444;
        }
        .metric-unit {
            font-size: 14px;
            color: #94a3b8;
            font-weight: 600;
        }
        .status-indicator {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status-indicator.green {
            background: #10b981;
            box-shadow: 0 0 10px rgba(16, 185, 129, 0.5);
        }
        .status-indicator.red {
            background: #ef4444;
            box-shadow: 0 0 10px rgba(239, 68, 68, 0.5);
            animation: pulse 1.5s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; transform: scale(1); }
            50% { opacity: 0.7; transform: scale(1.1); }
        }
        .card {
            background: linear-gradient(135deg, rgba(255,255,255,0.95) 0%, rgba(255,255,255,0.9) 100%);
            padding: 28px;
            border-radius: 20px;
            margin-bottom: 25px;
            box-shadow: 0 8px 32px rgba(0,0,0,0.1);
            backdrop-filter: blur(10px);
        }
        h2 {
            color: #1e293b;
            margin-bottom: 20px;
            font-size: 1.4em;
            font-weight: 700;
            display: flex;
            align-items: center;
        }
        h2 .icon {
            margin-right: 10px;
        }
        .chart-container {
            position: relative;
            height: 350px;
            margin-bottom: 10px;
        }
        .controls {
            display: flex;
            flex-wrap: wrap;
            gap: 12px;
            margin-bottom: 20px;
        }
        input, button {
            padding: 12px 18px;
            border-radius: 12px;
            border: 2px solid #e2e8f0;
            font-size: 14px;
            font-weight: 600;
            transition: all 0.3s;
        }
        input:focus {
            outline: none;
            border-color: #667eea;
            box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
        }
        button {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            cursor: pointer;
            border: none;
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.3);
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 16px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        .crash-status-banner {
            padding: 20px;
            border-radius: 16px;
            margin-bottom: 25px;
            text-align: center;
            font-weight: 700;
            font-size: 18px;
            transition: all 0.3s;
        }
        .crash-status-banner.no-crash {
            background: linear-gradient(135deg, #d1fae5 0%, #a7f3d0 100%);
            color: #047857;
            border: 2px solid #10b981;
        }
        .crash-status-banner.crash-detected {
            background: linear-gradient(135deg, #fee2e2 0%, #fecaca 100%);
            color: #991b1b;
            border: 2px solid #ef4444;
            animation: pulse 1.5s infinite;
        }
        .crash-timestamp {
            font-size: 14px;
            margin-top: 8px;
            opacity: 0.8;
        }
        .table-container {
            overflow-x: auto;
            max-height: 500px;
            overflow-y: auto;
        }
        table {
            width: 100%;
            border-collapse: separate;
            border-spacing: 0;
        }
        thead {
            background: linear-gradient(135deg, #f1f5f9 0%, #e2e8f0 100%);
            position: sticky;
            top: 0;
            z-index: 10;
        }
        th {
            padding: 14px;
            text-align: left;
            font-weight: 700;
            color: #1e293b;
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            border-bottom: 2px solid #cbd5e1;
        }
        td {
            padding: 14px;
            border-bottom: 1px solid #e2e8f0;
            color: #475569;
            font-size: 14px;
        }
        tr:hover {
            background: #f8fafc;
        }
        .severity-badge {
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 11px;
            font-weight: 700;
            text-transform: uppercase;
            display: inline-block;
        }
        .severity-low {
            background: #dbeafe;
            color: #1e40af;
        }
        .severity-medium {
            background: #fed7aa;
            color: #c2410c;
        }
        .severity-high {
            background: #fecaca;
            color: #991b1b;
        }
        @media (max-width: 768px) {
            .chart-container {
                height: 280px;
            }
            .metrics-grid {
                grid-template-columns: 1fr;
            }
            h1 {
                font-size: 1.5em;
            }
        }
    </style>
</head>
<body>
    <div id="connectionStatus" class="connection-status disconnected">● DISCONNECTED</div>
    
    <div class="container">
        <div class="header">
            <h1>⛑️ Smart Helmet IoT Analytics Dashboard</h1>
            <p class="subtitle">Real-Time MPU6050 Sensor Monitoring & Crash Detection System</p>
            <div class="timezone-badge">🇧🇩 Dhaka, Bangladesh (GMT+6)</div>
            <div class="current-time" id="currentTime">Loading time...</div>
        </div>

        <div class="metrics-grid">
            <div class="metric-card">
                <div class="metric-label">Resultant Acceleration</div>
                <div class="metric-value normal" id="metricAcc">0.00</div>
                <div class="metric-unit">g-force</div>
                <div style="margin-top: 12px;">
                    <span class="status-indicator green" id="accIndicator"></span>
                    <span style="font-size: 12px; color: #64748b; font-weight: 600;" id="accStatus">Normal</span>
                </div>
            </div>
            <div class="metric-card">
                <div class="metric-label">Angular Velocity</div>
                <div class="metric-value normal" id="metricGyro">0.00</div>
                <div class="metric-unit">deg/s</div>
                <div style="margin-top: 12px;">
                    <span class="status-indicator green" id="gyroIndicator"></span>
                    <span style="font-size: 12px; color: #64748b; font-weight: 600;" id="gyroStatus">Normal</span>
                </div>
            </div>
            <div class="metric-card">
                <div class="metric-label">Crash Detection Threshold</div>
                <div class="metric-value" style="color: #667eea; font-size: 28px;" id="metricThreshold">2.50g / 120°/s</div>
                <div class="metric-unit">Configurable limits</div>
            </div>
            <div class="metric-card">
                <div class="metric-label">Total Crashes Detected</div>
                <div class="metric-value warning" id="metricCrashCount">0</div>
                <div class="metric-unit">events recorded</div>
            </div>
        </div>

        <div class="crash-status-banner no-crash" id="crashStatusBanner">
            <span class="status-indicator green" id="crashIndicator"></span>
            <span id="crashStatusText">No Crash Detected</span>
            <div class="crash-timestamp" id="crashTimestamp" style="display:none;"></div>
        </div>

        <div class="card">
            <h2><span class="icon">⚙️</span> Threshold Configuration</h2>
            <div class="controls">
                <input type="number" id="accThreshold" placeholder="Acc Threshold (g)" step="0.1" min="1" max="10" value="2.5">
                <input type="number" id="gyroThreshold" placeholder="Gyro Threshold (°/s)" step="10" min="50" max="300" value="120">
                <input type="number" id="impactTime" placeholder="Impact Time (ms)" step="5" min="10" max="100" value="30">
                <button onclick="updateThresholds()">Update Thresholds</button>
                <button onclick="getCurrentSettings()">Refresh Settings</button>
            </div>
        </div>

        <div class="card">
            <h2><span class="icon">📈</span> Graph 1: Resultant Acceleration (G-Force)</h2>
            <div class="chart-container">
                <canvas id="accChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2><span class="icon">📈</span> Graph 2: Angular Velocity Magnitude</h2>
            <div class="chart-container">
                <canvas id="gyroChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2><span class="icon">📊</span> Graph 3: Combined Crash Analysis</h2>
            <div class="chart-container">
                <canvas id="combinedChart"></canvas>
            </div>
        </div>

        <div class="card">
            <h2><span class="icon">🧾</span> Crash History Table</h2>
            <div class="controls">
                <button onclick="refreshCrashHistory()">Refresh History</button>
                <button onclick="downloadCSV()">Download CSV</button>
                <button onclick="clearAllData()">Clear All Data</button>
            </div>
            <div class="table-container">
                <table id="crashTable">
                    <thead>
                        <tr>
                            <th>Crash ID</th>
                            <th>Date & Time</th>
                            <th>Peak G-Force</th>
                            <th>Peak Angular Velocity</th>
                            <th>Crash Severity</th>
                        </tr>
                    </thead>
                    <tbody id="crashTableBody">
                        <tr>
                            <td colspan="5" style="text-align: center; color: #94a3b8; padding: 40px;">No crashes detected yet</td>
                        </tr>
                    </tbody>
                </table>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let accChart, gyroChart, combinedChart;
        const maxDataPoints = 50;
        let accData = [];
        let gyroData = [];
        let timeLabels = [];
        let crashHistory = [];
        let accPointColors = [];
        let gyroPointColors = [];
        let accPointRadii = [];
        let gyroPointRadii = [];
        let accThresholdValue = 2.5;
        let gyroThresholdValue = 120.0;
        let crashIdCounter = 1;

        // Update current time display
        function updateCurrentTime() {
            fetch('/getCurrentTime')
            .then(response => response.text())
            .then(time => {
                document.getElementById('currentTime').textContent = '🕐 ' + time;
            })
            .catch(error => {
                console.error('Error fetching time:', error);
            });
        }

        // Calculate crash severity
        function calculateSeverity(acc, gyro) {
            const accFactor = acc / accThresholdValue;
            const gyroFactor = gyro / gyroThresholdValue;
            const severity = (accFactor + gyroFactor) / 2;
            
            if (severity >= 2.0) return 'High';
            if (severity >= 1.5) return 'Medium';
            return 'Low';
        }

        // Initialize charts with crash point highlighting and threshold lines
        function initCharts() {
            // Chart 1: Acceleration with threshold line
            const accCtx = document.getElementById('accChart').getContext('2d');
            accChart = new Chart(accCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Resultant G-Force',
                        data: accData,
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.05)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointRadius: accPointRadii,
                        pointBackgroundColor: accPointColors,
                        pointBorderColor: accPointColors,
                        pointHoverRadius: 8,
                        fill: true
                    }, {
                        label: 'Crash Threshold',
                        data: Array(maxDataPoints).fill(accThresholdValue),
                        borderColor: '#ef4444',
                        backgroundColor: 'rgba(239, 68, 68, 0.05)',
                        borderWidth: 2,
                        borderDash: [5, 5],
                        pointRadius: 0,
                        fill: false
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
                                text: 'g',
                                color: '#000000',
                                font: {
                                    size: 14,
                                    weight: 'bold'
                                }
                            },
                            grid: {
                                color: 'rgba(0, 0, 0, 0.05)'
                            },
                            ticks: {
                                color: '#1e293b',
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        x: {
                            title: {
                                display: true,
                                text: 'Time',
                                color: '#1e293b',
                                font: {
                                    weight: 'bold'
                                }
                            },
                            display: true,
                            grid: {
                                display: false
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: true,
                            labels: {
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        tooltip: {
                            callbacks: {
                                afterLabel: function(context) {
                                    const index = context.dataIndex;
                                    if (accPointColors[index] === '#ef4444') {
                                        return '🚨 CRASH DETECTED';
                                    }
                                    return '';
                                }
                            }
                        }
                    },
                    animation: {
                        duration: 300
                    }
                }
            });

            // Chart 2: Angular Velocity with threshold line
            const gyroCtx = document.getElementById('gyroChart').getContext('2d');
            gyroChart = new Chart(gyroCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'Angular Velocity Magnitude',
                        data: gyroData,
                        borderColor: '#10b981',
                        backgroundColor: 'rgba(16, 185, 129, 0.05)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointRadius: gyroPointRadii,
                        pointBackgroundColor: gyroPointColors,
                        pointBorderColor: gyroPointColors,
                        pointHoverRadius: 8,
                        fill: true
                    }, {
                        label: 'Crash Threshold',
                        data: Array(maxDataPoints).fill(gyroThresholdValue),
                        borderColor: '#ef4444',
                        backgroundColor: 'rgba(239, 68, 68, 0.05)',
                        borderWidth: 2,
                        borderDash: [5, 5],
                        pointRadius: 0,
                        fill: false
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
                                text: '°/s',
                                color: '#000000',
                                font: {
                                    size: 14,
                                    weight: 'bold'
                                }
                            },
                            grid: {
                                color: 'rgba(0, 0, 0, 0.05)'
                            },
                            ticks: {
                                color: '#1e293b',
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        x: {
                            title: {
                                display: true,
                                text: 'Time',
                                color: '#1e293b',
                                font: {
                                    weight: 'bold'
                                }
                            },
                            display: true,
                            grid: {
                                display: false
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: true,
                            labels: {
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        tooltip: {
                            callbacks: {
                                afterLabel: function(context) {
                                    const index = context.dataIndex;
                                    if (gyroPointColors[index] === '#ef4444') {
                                        return '🚨 CRASH DETECTED';
                                    }
                                    return '';
                                }
                            }
                        }
                    },
                    animation: {
                        duration: 300
                    }
                }
            });

            // Chart 3: Combined Analysis with dual Y-axis
            const combinedCtx = document.getElementById('combinedChart').getContext('2d');
            combinedChart = new Chart(combinedCtx, {
                type: 'line',
                data: {
                    labels: timeLabels,
                    datasets: [{
                        label: 'G-Force',
                        data: accData,
                        borderColor: '#3b82f6',
                        backgroundColor: 'rgba(59, 130, 246, 0.1)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointRadius: accPointRadii,
                        pointBackgroundColor: accPointColors,
                        pointBorderColor: accPointColors,
                        yAxisID: 'y',
                        fill: false
                    }, {
                        label: 'Angular Velocity',
                        data: gyroData,
                        borderColor: '#10b981',
                        backgroundColor: 'rgba(16, 185, 129, 0.1)',
                        borderWidth: 3,
                        tension: 0.4,
                        pointRadius: gyroPointRadii,
                        pointBackgroundColor: gyroPointColors,
                        pointBorderColor: gyroPointColors,
                        yAxisID: 'y1',
                        fill: false
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    interaction: {
                        mode: 'index',
                        intersect: false,
                    },
                    scales: {
                        y: {
                            type: 'linear',
                            display: true,
                            position: 'left',
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'G-Force (g)',
                                color: '#3b82f6',
                                font: {
                                    size: 14,
                                    weight: 'bold'
                                }
                            },
                            grid: {
                                color: 'rgba(59, 130, 246, 0.1)'
                            },
                            ticks: {
                                color: '#3b82f6',
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        y1: {
                            type: 'linear',
                            display: true,
                            position: 'right',
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Angular Velocity (°/s)',
                                color: '#10b981',
                                font: {
                                    size: 14,
                                    weight: 'bold'
                                }
                            },
                            grid: {
                                drawOnChartArea: false,
                            },
                            ticks: {
                                color: '#10b981',
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        x: {
                            title: {
                                display: true,
                                text: 'Time',
                                color: '#1e293b',
                                font: {
                                    weight: 'bold'
                                }
                            },
                            display: true,
                            grid: {
                                display: false
                            }
                        }
                    },
                    plugins: {
                        legend: {
                            display: true,
                            labels: {
                                font: {
                                    weight: '600'
                                }
                            }
                        },
                        tooltip: {
                            callbacks: {
                                afterLabel: function(context) {
                                    const index = context.dataIndex;
                                    if (accPointColors[index] === '#ef4444' || gyroPointColors[index] === '#ef4444') {
                                        return '🚨 CRASH DETECTED';
                                    }
                                    return '';
                                }
                            }
                        }
                    },
                    animation: {
                        duration: 300
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

        // Update sensor data on charts with RED crash markers
        function updateSensorData(data) {
            // Update metric cards
            const metricAcc = document.getElementById('metricAcc');
            const metricGyro = document.getElementById('metricGyro');
            const accIndicator = document.getElementById('accIndicator');
            const gyroIndicator = document.getElementById('gyroIndicator');
            const accStatus = document.getElementById('accStatus');
            const gyroStatus = document.getElementById('gyroStatus');
            
            metricAcc.textContent = data.acc.toFixed(2);
            metricGyro.textContent = data.gyro.toFixed(2);
            
            // Update status indicators
            if (data.acc > accThresholdValue) {
                metricAcc.className = 'metric-value danger';
                accIndicator.className = 'status-indicator red';
                accStatus.textContent = 'Crash Level';
            } else if (data.acc > accThresholdValue * 0.7) {
                metricAcc.className = 'metric-value warning';
                accIndicator.className = 'status-indicator red';
                accStatus.textContent = 'Warning';
            } else {
                metricAcc.className = 'metric-value normal';
                accIndicator.className = 'status-indicator green';
                accStatus.textContent = 'Normal';
            }
            
            if (data.gyro > gyroThresholdValue) {
                metricGyro.className = 'metric-value danger';
                gyroIndicator.className = 'status-indicator red';
                gyroStatus.textContent = 'Crash Level';
            } else if (data.gyro > gyroThresholdValue * 0.7) {
                metricGyro.className = 'metric-value warning';
                gyroIndicator.className = 'status-indicator red';
                gyroStatus.textContent = 'Warning';
            } else {
                metricGyro.className = 'metric-value normal';
                gyroIndicator.className = 'status-indicator green';
                gyroStatus.textContent = 'Normal';
            }
            
            // Update charts
            if (accData.length >= maxDataPoints) {
                accData.shift();
                gyroData.shift();
                timeLabels.shift();
                accPointColors.shift();
                gyroPointColors.shift();
                accPointRadii.shift();
                gyroPointRadii.shift();
            }
            
            accData.push(data.acc);
            gyroData.push(data.gyro);
            
            const now = new Date();
            const timeStr = now.getHours().toString().padStart(2, '0') + ':' + 
                           now.getMinutes().toString().padStart(2, '0') + ':' + 
                           now.getSeconds().toString().padStart(2, '0');
            timeLabels.push(timeStr);
            
            // Mark crash points in RED with larger radius
            if (data.crash) {
                accPointColors.push('#ef4444');
                gyroPointColors.push('#ef4444');
                accPointRadii.push(10);
                gyroPointRadii.push(10);
                
                // Update crash status banner
                const banner = document.getElementById('crashStatusBanner');
                const indicator = document.getElementById('crashIndicator');
                const statusText = document.getElementById('crashStatusText');
                const timestamp = document.getElementById('crashTimestamp');
                
                banner.className = 'crash-status-banner crash-detected';
                indicator.className = 'status-indicator red';
                statusText.textContent = '🚨 Crash Detected!';
                timestamp.style.display = 'block';
                timestamp.textContent = 'Detected at: ' + timeStr;
                
                // Reset after 5 seconds
                setTimeout(() => {
                    banner.className = 'crash-status-banner no-crash';
                    indicator.className = 'status-indicator green';
                    statusText.textContent = 'No Crash Detected';
                    timestamp.style.display = 'none';
                }, 5000);
            } else {
                accPointColors.push('#3b82f6');
                gyroPointColors.push('#10b981');
                accPointRadii.push(0);
                gyroPointRadii.push(0);
            }
            
            // Update all charts
            accChart.data.datasets[0].pointBackgroundColor = accPointColors;
            accChart.data.datasets[0].pointBorderColor = accPointColors;
            accChart.data.datasets[0].pointRadius = accPointRadii;
            accChart.data.datasets[1].data = Array(accData.length).fill(accThresholdValue);
            accChart.update('none');
            
            gyroChart.data.datasets[0].pointBackgroundColor = gyroPointColors;
            gyroChart.data.datasets[0].pointBorderColor = gyroPointColors;
            gyroChart.data.datasets[0].pointRadius = gyroPointRadii;
            gyroChart.data.datasets[1].data = Array(gyroData.length).fill(gyroThresholdValue);
            gyroChart.update('none');
            
            combinedChart.data.datasets[0].pointBackgroundColor = accPointColors;
            combinedChart.data.datasets[0].pointBorderColor = accPointColors;
            combinedChart.data.datasets[0].pointRadius = accPointRadii;
            combinedChart.data.datasets[1].pointBackgroundColor = gyroPointColors;
            combinedChart.data.datasets[1].pointBorderColor = gyroPointColors;
            combinedChart.data.datasets[1].pointRadius = gyroPointRadii;
            combinedChart.update('none');
        }

        // Handle crash event
        function handleCrashEvent(data) {
            document.getElementById('metricCrashCount').textContent = data.count;
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
            
            accThresholdValue = data.acc;
            gyroThresholdValue = data.gyro;
            
            document.getElementById('metricThreshold').textContent = 
                data.acc.toFixed(2) + 'g / ' + data.gyro.toFixed(0) + '°/s';
            
            // Update threshold lines in charts
            if (accChart) {
                accChart.data.datasets[1].data = Array(maxDataPoints).fill(data.acc);
                accChart.update('none');
            }
            if (gyroChart) {
                gyroChart.data.datasets[1].data = Array(maxDataPoints).fill(data.gyro);
                gyroChart.update('none');
            }
        }

        // Refresh crash history with table format
        function refreshCrashHistory() {
            fetch('/getCrashHistory')
            .then(response => response.json())
            .then(data => {
                const tbody = document.getElementById('crashTableBody');
                if (data.crashes && data.crashes.length > 0) {
                    tbody.innerHTML = '';
                    
                    // Sort crashes by timestamp (most recent first)
                    data.crashes.sort((a, b) => b.timestamp - a.timestamp);
                    
                    data.crashes.forEach((crash, index) => {
                        const severity = calculateSeverity(crash.acc, crash.gyro);
                        const severityClass = 'severity-' + severity.toLowerCase();
                        
                        const row = document.createElement('tr');
                        row.innerHTML = `
                            <td><strong>#${crashIdCounter - index}</strong></td>
                            <td>${crash.time}</td>
                            <td><strong>${crash.acc.toFixed(2)} g</strong></td>
                            <td><strong>${crash.gyro.toFixed(2)} °/s</strong></td>
                            <td><span class="severity-badge ${severityClass}">${severity}</span></td>
                        `;
                        tbody.appendChild(row);
                    });
                    
                    crashIdCounter = data.crashes.length + 1;
                    document.getElementById('metricCrashCount').textContent = data.crashes.length;
                } else {
                    tbody.innerHTML = '<tr><td colspan="5" style="text-align: center; color: #94a3b8; padding: 40px;">No crashes detected yet</td></tr>';
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
                    crashIdCounter = 1;
                    refreshCrashHistory();
                    document.getElementById('metricCrashCount').textContent = '0';
                });
            }
        }

        // Initialize on page load
        window.onload = function() {
            initCharts();
            initWebSocket();
            getCurrentSettings();
            refreshCrashHistory();
            updateCurrentTime();
            
            // Update current time every second
            setInterval(updateCurrentTime, 1000);
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
  
  // Find all crash data points with Dhaka time
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].isCrash && dataHistory[i].timestamp > 0) {
      JsonObject crash = crashes.createNestedObject();
      crash["timestamp"] = dataHistory[i].timestamp;
      crash["acc"] = dataHistory[i].accMag;
      crash["gyro"] = dataHistory[i].gyroMag;
      crash["time"] = dataHistory[i].timeString;
    }
  }
  
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleDownloadCSV() {
  String csv = "Timestamp (ms),Dhaka Time,Acceleration (g),Gyroscope (deg/s),Crash Detected\n";
  
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    if (dataHistory[i].timestamp > 0) {
      csv += String(dataHistory[i].timestamp) + ",";
      csv += dataHistory[i].timeString + ",";
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
    dataHistory[i].timeString = "";
  }
  dataIndex = 0;
  crashCount = 0;
  
  Serial.println("All data cleared");
  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void handleGetCurrentTime() {
  String currentTime = getDhakaTime();
  server.send(200, "text/plain", currentTime);
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
  Serial.println("   MPU TEST WITH DHAKA TIMEZONE         ");
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
    
    // Initialize NTP Client for Dhaka timezone
    Serial.println("🕐 Synchronizing time with NTP server...");
    timeClient.begin();
    timeClient.update();
    
    int ntpAttempts = 0;
    while (!timeClient.isTimeSet() && ntpAttempts < 10) {
      delay(500);
      timeClient.update();
      Serial.print(".");
      ntpAttempts++;
    }
    
    if (timeClient.isTimeSet()) {
      Serial.println("\n✅ Time synchronized!");
      Serial.print("🇧🇩 Dhaka Time: ");
      Serial.println(getDhakaTime());
    } else {
      Serial.println("\n⚠️ Time sync failed, will retry in background");
    }
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
  server.on("/getCurrentTime", handleGetCurrentTime);
  
  // Start web server
  server.begin();
  Serial.println("✅ Web server started");
  
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("✅ WebSocket server started");
  
  Serial.println("\n========================================");
  Serial.println("   SYSTEM READY - TESTING MODE         ");
  Serial.println("   Timezone: Dhaka, Bangladesh (GMT+6)  ");
  Serial.println("========================================\n");
}

/* ================= LOOP ================= */

void loop() {
  // Handle web server
  server.handleClient();
  webSocket.loop();
  
  // Update NTP time periodically
  static unsigned long lastNTPUpdate = 0;
  if (millis() - lastNTPUpdate > 60000) {  // Update every minute
    timeClient.update();
    lastNTPUpdate = millis();
  }
  
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
  
  // Print to serial with Dhaka time
  Serial.print("["); Serial.print(getDhakaTime()); Serial.print("] ");
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
        Serial.print(") at ");
        Serial.print(getDhakaTime());
        Serial.println();
        
        // Send crash event via WebSocket
        StaticJsonDocument<300> crashDoc;
        crashDoc["type"] = "crash";
        crashDoc["count"] = crashCount;
        crashDoc["acc"] = accMag;
        crashDoc["gyro"] = gyroMag;
        crashDoc["timestamp"] = millis();
        crashDoc["time"] = getDhakaTime();
        
        String crashJson;
        serializeJson(crashDoc, crashJson);
        webSocket.broadcastTXT(crashJson);
        
        // Set crash reset time for non-blocking delay
        crashResetTime = millis() + CRASH_RESET_DELAY;
      }
    }
  } else {
    impactStart = 0;
  }
  
  // Reset crash flag after timeout (non-blocking)
  if (crashDetected && crashResetTime > 0 && millis() >= crashResetTime) {
    crashDetected = false;
    crashResetTime = 0;
  }
  
  Serial.println();
  
  // Store data point with Dhaka time
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
