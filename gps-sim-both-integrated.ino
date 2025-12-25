#include <TinyGPS++.h>
#include <HardwareSerial.h>

// ---------- GPS ----------
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);   // UART2 for GPS
#define GPS_RX 16
#define GPS_TX 17

// ---------- SIM800L ----------
HardwareSerial sim800(1);      // UART1 for SIM800L
#define SIM_RX 26
#define SIM_TX 27

String phoneNumber = "+880XXXXXXXXXX";  // <-- PUT YOUR NUMBER

void setup() {
  Serial.begin(115200);
  delay(2000);

  // GPS
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // SIM800L
  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(6000);   // SIM800L boot time

  Serial.println("System Ready");
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isValid()) {
    float lat = gps.location.lat();
    float lng = gps.location.lng();

    Serial.println("GPS FIX OK");
    Serial.print("Lat: "); Serial.println(lat, 6);
    Serial.print("Lng: "); Serial.println(lng, 6);

    sendLocationSMS(lat, lng);

    delay(60000); // Send once per minute
  } 
  else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(2000);
}

void sendLocationSMS(float lat, float lng) {
  String message = "🚨 GPS LOCATION 🚨\n";
  message += "Lat: " + String(lat, 6) + "\n";
  message += "Lng: " + String(lng, 6) + "\n\n";
  message += "Google Maps:\n";
  message += "https://maps.google.com/?q=";
  message += String(lat, 6) + "," + String(lng, 6);

  sim800.println("AT+CMGF=1");
  delay(500);

  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(500);

  sim800.print(message);
  delay(300);

  sim800.write(26); // CTRL+Z
  delay(5000);

  Serial.println("SMS SENT");
}
