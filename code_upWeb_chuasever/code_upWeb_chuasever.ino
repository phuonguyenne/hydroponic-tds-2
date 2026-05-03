#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= CẤU HÌNH =================
const char* ssid = "Phuong Uyen";
const char* password = "0914859672hong.";

String serverInsert = "https://hydroponic-tds-1-production.up.railway.app/insert.php";
String serverMode   = "https://hydroponic-tds-1-production.up.railway.app/get-mode.php";
// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;   // +7 VN

// PIN
#define TDS_PIN 5
#define ONE_WIRE_BUS 4

// SENSOR
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// TDS calibration
#define SCOUNT 30
float a = 2.2358;
float b = -3183.67;

// CONTROL
unsigned long lastMeasure = 0;
unsigned long lastSend = 0;
unsigned long lastSync = 0;
String mode = "non";

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== HYDROPONIC MONITOR ESP32 STARTING ===");

  sensors.begin();
  analogReadResolution(12);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  configTime(gmtOffset_sec, 0, ntpServer);
  Serial.println("NTP configured.");

  struct tm timeinfo;
  for(int i = 0; i < 20; i++) {
    if(getLocalTime(&timeinfo, 1000)) {
      Serial.println("Time synced successfully!");
      break;
    }
    Serial.print(".");
    delay(500);
  }
}

// ================= LOOP =================
void loop() {
  // Sync NTP mỗi 6 giờ
  if (millis() - lastSync > 21600000UL) {
    Serial.println("Re-syncing NTP...");
    configTime(gmtOffset_sec, 0, ntpServer);
    lastSync = millis();
  }

  // Đo mỗi 5 giây
  if (millis() - lastMeasure > 5000) {
    lastMeasure = millis();

    float tds = readTDS();
    float temp = readTempCalibrated();   // Nhiệt độ đã calib

    mode = getMode();

    Serial.println("=== MEASUREMENT ===");
    Serial.print("TDS: ");  Serial.println(tds);
    Serial.print("Temp (calib): "); Serial.println(temp);
    Serial.print("Mode: "); Serial.println(mode);

    // Chỉ gửi lên server mỗi 30 giây
    if (millis() - lastSend > 30000) {
      sendData(tds, temp);
      lastSend = millis();
    }
  }

  delay(10);
}

// ================= READ TDS =================
float readTDS() {
  float sum = 0;
  for (int i = 0; i < SCOUNT; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  float raw = sum / SCOUNT;
  float tds = a * raw + b;
  if (tds < 0 || raw < 100) tds = 0;
  return tds;
}

// ================= READ TEMP + CALIB =================
float readTempCalibrated() {
  sensors.requestTemperatures();
  float x = sensors.getTempCByIndex(0);   // x là giá trị thô từ cảm biến

  if (x == DEVICE_DISCONNECTED_C) {
    Serial.println("Temp sensor error!");
    return 25.0;
  }

  // Công thức calib bạn cung cấp
  float tempCalib = -0.027 * x * x + 2.473 * x - 19.535;
  return tempCalib;
}

// ================= GET MODE =================
String getMode() {
  if (WiFi.status() != WL_CONNECTED) return "non";

  HTTPClient http;
  http.begin(serverMode);
  int code = http.GET();
  String m = "non";

  if (code > 0) {
    m = http.getString();
    m.trim();
  } else {
    Serial.print("GET mode failed, code: "); Serial.println(code);
  }
  http.end();
  return m;
}

// ================= SEND DATA =================
void sendData(float tds, float temp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, skip send");
    return;
  }

  HTTPClient http;
  http.begin(serverInsert);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "tds=" + String(tds, 1) + "&temp=" + String(temp, 1);

  int code = http.POST(postData);

  Serial.print("HTTP POST to insert.php: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    response.trim();
    if (response.length() > 0) Serial.println("Response: " + response);
  }
  http.end();
}