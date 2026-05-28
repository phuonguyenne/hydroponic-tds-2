#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WEB =====
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
const char* kServerBase = "https://hydroponic-tds-2-production.up.railway.app";
String serverInsert = String(kServerBase) + "/insert.php";
String serverMode   = String(kServerBase) + "/get-mode.php";
const char* UPLOAD_API_KEY = ""; // de "" neu server khong bat key
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;

// ===== DÁN KẾT QUẢ TỪ 2 FILE CALIB RIÊNG =====
float OFFSET_B = 0.0f;   // tu calib_tds_offset_30
float flowA = 0.0f;      // ml/s tu calib_pumps_only
float flowB = 0.0f;      // ml/s tu calib_pumps_only
float ppm_per_ml = 40.0f; // tu quy trinh PPM START/END neu can auto run

// ===== PINS =====
#define ONE_WIRE_BUS 4
#define TDS_PIN 5
#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16
#define SDA_RTC 1
#define SCL_RTC 2

RTC_DS3231 rtc;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperature = 25.0f;
String mode = "non";
bool running = false;
bool wifiReady = false;
bool webPostEnabled = false;

unsigned long lastMeasure = 0;
unsigned long lastSync = 0;

void pumpA_on() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}
void pumpA_off() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}
void pumpB_on() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void pumpB_off() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
void stopAll() {
  pumpA_off();
  pumpB_off();
}

bool isWorkingTime() {
  DateTime now = rtc.now();
  int nowMin = now.hour() * 60 + now.minute();
  return (nowMin >= 360 && nowMin <= 970); // 06:00 -> 16:10
}

float readTempRaw() {
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);
  if (t < -50 || t > 85) return 25.0f;
  return t;
}

// Cong thuc calib nhiet do (giu giong code done sever)
float readTempCalibrated() {
  sensors.requestTemperatures();
  float x = sensors.getTempCByIndex(0);
  if (x == DEVICE_DISCONNECTED_C) {
    Serial.println("Temp sensor error!");
    return 25.0f;
  }
  return -0.027f * x * x + 2.473f * x - 19.535f;
}

float readRawAvg30() {
  float sum = 0.0f;
  for (int i = 0; i < 30; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  return sum / 30.0f;
}

float readTDS_NoTempComp() {
  float raw = readRawAvg30();
  float tds = raw + OFFSET_B; // a = 1
  if (tds < 0 || raw < 100) tds = 0;
  return tds;
}

String getMode() {
  if (WiFi.status() != WL_CONNECTED) return "non";

  HTTPClient http;
  http.begin(serverMode);
  int code = http.GET();
  String m = "non";

  if (code > 0) {
    m = http.getString();
    m.trim();
    if (m != "non" && m != "truongthanh") m = "non";
  } else {
    Serial.print("GET mode failed, code: ");
    Serial.println(code);
  }
  http.end();
  return m;
}

void sendData(float tds, float temp) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, skip send");
    return;
  }

  HTTPClient http;
  http.begin(serverInsert);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  if (UPLOAD_API_KEY[0] != '\0') {
    http.addHeader("X-Api-Key", UPLOAD_API_KEY);
  }

  String postData = "tds=" + String(tds, 1) + "&temp=" + String(temp, 1);
  int code = http.POST(postData);

  Serial.print("HTTP POST insert.php: ");
  Serial.println(code);

  if (code > 0) {
    String response = http.getString();
    response.trim();
    if (response.length() > 0) Serial.println("Response: " + response);
  }
  http.end();
}

void setupWiFiNtpOnce() {
  if (wifiReady) return;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  configTime(gmtOffset_sec, 0, ntpServer);
  struct tm timeinfo;
  for (int i = 0; i < 20; i++) {
    if (getLocalTime(&timeinfo, 1000)) {
      Serial.println("Time synced successfully!");
      break;
    }
    delay(500);
  }
  wifiReady = true;
}

void doseEqualML(float ml) {
  if (flowA <= 0 || flowB <= 0) {
    Serial.println("Flow A/B chua calib.");
    return;
  }

  if (ml < 1) ml = 1;
  if (ml > 20) ml = 20;

  float timeA = ml / flowA;
  float timeB = ml / flowB;

  pumpA_on();
  pumpB_on();

  unsigned long start = millis();
  while (true) {
    if (Serial.available()) {
      String cmd = Serial.readStringUntil('\n');
      cmd.trim();
      if (cmd == "STOP") {
        running = false;
        stopAll();
        Serial.println("STOP");
        return;
      }
    }

    unsigned long dt = millis() - start;
    if (dt >= (unsigned long) (timeA * 1000.0f)) pumpA_off();
    if (dt >= (unsigned long) (timeB * 1000.0f)) pumpB_off();
    if (dt >= (unsigned long) (max(timeA, timeB) * 1000.0f)) break;
  }

  stopAll();
}

void autoRun() {
  if (!isWorkingTime()) return;

  DateTime now = rtc.now();
  int nowMin = now.hour() * 60 + now.minute();
  int cycle = (nowMin - 360) % 40;
  if (cycle < 0) cycle += 40;
  int sec = now.second();
  unsigned long t_ms = (cycle * 60 + sec) * 1000UL;

  // 10 phut ON
  if (cycle < 10) {
    // cho tron 2 phut dau
    if (t_ms < 120000) return;

    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 30000) return;
    lastCheck = millis();

    float ppm = readTDS_NoTempComp();
    Serial.print("PPM: ");
    Serial.println(ppm);

    // giu logic goc: neu ppm < 550 thi bom huong ve moc 600
    if (ppm < 550) {
      float deficit = 600 - ppm;
      if (ppm_per_ml > 0.001f) {
        float ml = (deficit / ppm_per_ml) * 0.3f;
        Serial.print("Dose ML: ");
        Serial.println(ml);
        doseEqualML(ml);
      } else {
        Serial.println("ppm_per_ml chua dung, bo qua bom tu dong.");
      }
    }
  }
}

void printHelp() {
  Serial.println("Lenh: WEB ON | WEB OFF | SHOW | RUN | STOP");
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== FINAL NO TEMP COMP ===");

  pinMode(TDS_PIN, INPUT);
  analogReadResolution(12);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopAll();

  sensors.begin();
  Wire.begin(SDA_RTC, SCL_RTC);
  if (!rtc.begin()) {
    Serial.println("RTC FAIL");
    while (1) {}
  }

  // ===== CHỈ MỞ 1 LẦN ĐỂ SET GIỜ RTC =====
  // rtc.adjust(DateTime(2026, 5, 8, 15, 30, 0));

  printHelp();
  Serial.println("Nho dan OFFSET_B / flowA / flowB / ppm_per_ml tu 2 file calib rieng.");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "WEB ON") {
      setupWiFiNtpOnce();
      webPostEnabled = true;
      lastMeasure = millis();
      Serial.println("WEB POST ON");
    } else if (cmd == "WEB OFF") {
      webPostEnabled = false;
      Serial.println("WEB POST OFF");
    } else if (cmd == "SHOW") {
      float tds = readTDS_NoTempComp();
      temperature = readTempCalibrated();
      Serial.print("TempCalib=");
      Serial.print(temperature, 2);
      Serial.print(" | TDS=");
      Serial.print(tds, 2);
      Serial.print(" | OFFSET_B=");
      Serial.println(OFFSET_B, 3);
    } else if (cmd == "RUN") {
      running = true;
      Serial.println("RUNNING");
    } else if (cmd == "STOP") {
      running = false;
      stopAll();
      Serial.println("STOP");
    } else {
      printHelp();
    }
  }

  if (webPostEnabled && wifiReady) {
    if (millis() - lastSync > 21600000UL) {
      configTime(gmtOffset_sec, 0, ntpServer);
      lastSync = millis();
    }

    if (millis() - lastMeasure > 5000) {
      lastMeasure = millis();

      float tdsWeb = readTDS_NoTempComp();
      float tempWeb = readTempCalibrated();
      mode = getMode();

      Serial.println("=== POST WEB ===");
      Serial.print("TDS: ");
      Serial.println(tdsWeb);
      Serial.print("TempCalib: ");
      Serial.println(tempWeb);
      Serial.print("Mode: ");
      Serial.println(mode);

      sendData(tdsWeb, tempWeb);
    }
  }

  if (running) autoRun();
  delay(10);
}

