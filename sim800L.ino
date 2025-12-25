#include <HardwareSerial.h>

HardwareSerial sim800(2);   // UART2

#define SIM_RX 16   // ESP32 RX2 <- SIM800 TX
#define SIM_TX 17   // ESP32 TX2 -> SIM800 RX

void sendCmd(const char* cmd, unsigned long waitMs = 1500) {
  sim800.println(cmd);
  delay(waitMs);
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("SIM800L SMS TEST START");

  sim800.begin(9600, SERIAL_8N1, SIM_RX, SIM_TX);
  delay(6000); // allow SIM800L to boot fully

  // BASIC INIT
  sendCmd("AT");
  sendCmd("ATE0");
  sendCmd("AT+CPIN?");
  sendCmd("AT+CSQ");
  sendCmd("AT+CREG?");
  sendCmd("AT+CMGF=1");          // SMS text mode
  sendCmd("AT+CSCS=\"GSM\"");    // GSM charset

  // -------- SEND SMS --------
  Serial.println("Sending SMS...");

  sim800.print("AT+CMGS=\"+8801747213525\"\r");  // <-- YOUR NUMBER
  delay(1000);

  sim800.print("🚨 TEST SMS 🚨\nSIM800L is WORKING.\nESP32 project ready.");
  delay(500);

  sim800.write(26);  // CTRL+Z
  delay(5000);

  Serial.println("SMS SEND COMMAND DONE");
}

void loop() {
  // nothing
}
