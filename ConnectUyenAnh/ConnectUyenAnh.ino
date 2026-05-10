
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

#include <Wire.h>
#include "RTClib.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>

// ----- Web (giống code_upWeb_donesever) -----
const char* ssid = "Phuong Uyen";
const char* password = "0914859672hong.";
const char* kServerBase = "https://hydroponic-tds-2-production.up.railway.app";
String serverInsert = String(kServerBase) + "/insert.php";
String serverMode   = String(kServerBase) + "/get-mode.php";

/* Trùng với biến môi trường UPLOAD_API_KEY trên Railway (insert.php). Để "" nếu server không bật key.
 * Không đẩy khóa thật lên Git công khai — có thể chỉnh cục bộ trước khi nạp. */
const char* UPLOAD_API_KEY = "";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;

#define WEB_PPM_AVG_N 30

unsigned long lastMeasure = 0;
unsigned long lastSync = 0;
String mode = "non";

bool wifiReady = false;
bool webPostEnabled = false;

// ================= RTC =================
RTC_DS3231 rtc;

#define ONE_WIRE_BUS 4
#define TDS_PIN 5
#define RAW_CALIB_N 30

#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16
#define SDA_RTC 1
#define SCL_RTC 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperature = 25;

Preferences prefs;

float a = 1;
float b = 0;
float raw1, raw2;
bool tdsReady = false;

float flowA = 0, flowB = 0;
bool flowReady = false;
float ppm_per_ml = 0;
bool ppmReady = false;
float ppm0, ppm1;

bool running = false;

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

void saveCalib() {
  prefs.begin("calib", false);
  prefs.putFloat("a", a);
  prefs.putFloat("b", b);
  prefs.putFloat("flowA", flowA);
  prefs.putFloat("flowB", flowB);
  prefs.putFloat("ppmml", ppm_per_ml);
  prefs.end();
}

void loadCalib() {
  prefs.begin("calib", true);
  a = prefs.getFloat("a", 1);
  b = prefs.getFloat("b", 0);
  flowA = prefs.getFloat("flowA", 0);
  flowB = prefs.getFloat("flowB", 0);
  ppm_per_ml = prefs.getFloat("ppmml", 0);
  prefs.end();

  if (a != 1 || b != 0) tdsReady = true;
  if (flowA > 0 && flowB > 0)
    flowReady = true;
  if (ppm_per_ml > 0)
    ppmReady = true;
}

float readTemp() {
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);
  if (t < -50 || t > 80) return 25;
  return t;
}

float readTempCalibrated() {
  sensors.requestTemperatures();
  float x = sensors.getTempCByIndex(0);
  if (x == DEVICE_DISCONNECTED_C) {
    Serial.println("Temp sensor error!");
    return 25.0;
  }
  return -0.027 * x * x + 2.473 * x - 19.535;
}

float readRaw() {
  int sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(TDS_PIN);
    delay(5);
  }
  return sum / 20.0;
}

float readRawCalib30() {
  float sum = 0;
  for (int i = 0; i < RAW_CALIB_N; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  return sum / (float)RAW_CALIB_N;
}

float ppmFromRawWithGlobalTemp(float raw) {
  float tds = a * raw + b;
  float comp = 1 + 0.02f * (temperature - 25.0f);
  if (comp > 0.01f) tds /= comp;
  if (tds < 0 || raw < 100) tds = 0;
  return tds;
}

float readTDS() {
  temperature = readTempCalibrated();
  float raw = readRaw();
  return ppmFromRawWithGlobalTemp(raw);
}

float readAveragePPMCalibratedForWeb() {
  temperature = readTempCalibrated();
  float sum = 0;
  for (int i = 0; i < WEB_PPM_AVG_N; i++) {
    sum += ppmFromRawWithGlobalTemp((float)analogRead(TDS_PIN));
    delay(10);
  }
  return sum / (float)WEB_PPM_AVG_N;
}

void setupWiFiNtpOnce() {
  if (wifiReady) return;
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK: " + WiFi.localIP().toString());

  configTime(gmtOffset_sec, 0, ntpServer);
  struct tm timeinfo;
  for (int i = 0; i < 20; i++) {
    if (getLocalTime(&timeinfo, 1000)) {
      Serial.println("NTP ok");
      break;
    }
    delay(500);
  }
  wifiReady = true;
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
    Serial.print("GET mode failed: ");
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
    String r = http.getString();
    r.trim();
    if (r.length() > 0) Serial.println("Response: " + r);
  }
  http.end();
}

void doseEqualML(float ml) {
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
    unsigned long t = millis() - start;
    if (t >= timeA * 1000) pumpA_off();
    if (t >= timeB * 1000) pumpB_off();
    if (t >= max(timeA, timeB) * 1000) break;
  }
  stopAll();
}

bool isWorkingTime() {
  DateTime now = rtc.now();
  int nowMin = now.hour() * 60 + now.minute();
  return (nowMin >= 360 && nowMin <= 970);
}

void handleSerial() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd == "CAL 1000") {
    Serial.println("Nhung vao dung dich 1000 (TB 30 mau RAW, khong gui web)");
    delay(5000);
    raw1 = readRawCalib30();
    Serial.print("RAW1: ");
    Serial.println(raw1);
  }

  else if (cmd == "CAL 1382") {
    Serial.println("Nhung vao dung dich 1382 (TB 30 mau RAW, khong gui web)");
    delay(5000);
    raw2 = readRawCalib30();
    Serial.print("RAW2: ");
    Serial.println(raw2);
    a = (1382.0 - 1000.0) / (raw2 - raw1);
    b = 1000.0 - a * raw1;
    tdsReady = true;
    saveCalib();
    Serial.println("TDS CALIB DONE");
  }

  else if (cmd == "WEB ON") {
    if (!tdsReady || !flowReady || !ppmReady) {
      Serial.println("CHUA CALIB DU (TDS + bom + PPM)");
      return;
    }
    setupWiFiNtpOnce();
    webPostEnabled = true;
    lastMeasure = millis();
    Serial.println("POST web: BAT (5s/lan, TB 30 ppm calib). WEB OFF de tat.");
  }

  else if (cmd == "WEB OFF") {
    webPostEnabled = false;
    Serial.println("POST web: TAT");
  }

  else if (cmd.startsWith("FLOW A")) {
    int sec = cmd.substring(7).toInt();
    Serial.println("Bom A dang chay");
    pumpA_on();
    delay(sec * 1000);
    pumpA_off();
    Serial.println("Nhap: ML A xx");
  }

  else if (cmd.startsWith("ML A")) {
    float ml = cmd.substring(5).toFloat();
    flowA = ml / 10.0;
    flowReady = (flowA > 0 && flowB > 0);
    saveCalib();
    Serial.print("FLOW A: ");
    Serial.println(flowA);
  }

  else if (cmd.startsWith("FLOW B")) {
    int sec = cmd.substring(7).toInt();
    Serial.println("Bom B dang chay");
    pumpB_on();
    delay(sec * 1000);
    pumpB_off();
    Serial.println("Nhap: ML B xx");
  }

  else if (cmd.startsWith("ML B")) {
    float ml = cmd.substring(5).toFloat();
    flowB = ml / 10.0;
    flowReady = (flowA > 0 && flowB > 0);
    saveCalib();
    Serial.print("FLOW B: ");
    Serial.println(flowB);
  }

  else if (cmd == "PPM START") {
    ppm0 = readTDS();
    Serial.print("PPM0: ");
    Serial.println(ppm0);
  }

  else if (cmd.startsWith("PPM DOSE")) {
    float ml = cmd.substring(9).toFloat();
    doseEqualML(ml);
    Serial.println("Da bom xong");
  }

  else if (cmd == "PPM END") {
    Serial.println("Cho tron 90s");
    delay(90000);
    ppm1 = readTDS();
    Serial.print("PPM1: ");
    Serial.println(ppm1);
    ppm_per_ml = (ppm1 - ppm0) / 10.0;
    ppmReady = true;
    saveCalib();
    Serial.print("PPM/ML: ");
    Serial.println(ppm_per_ml);
  }

  else if (cmd == "RUN") {
    if (!tdsReady || !flowReady || !ppmReady) {
      Serial.println("CHUA CALIB");
      return;
    }
    running = true;
    Serial.println("RUNNING");
  }

  else if (cmd == "STOP") {
    running = false;
    stopAll();
    Serial.println("STOP");
  }

  else if (cmd == "SHOW") {
    float ppm = readTDS();
    DateTime now = rtc.now();
    Serial.println("====== STATUS ======");
    Serial.print("Time: ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());
    Serial.print("Temp (calib): ");
    Serial.println(temperature);
    Serial.print("PPM: ");
    Serial.println(ppm);
    Serial.print("flowA: ");
    Serial.println(flowA);
    Serial.print("flowB: ");
    Serial.println(flowB);
    Serial.print("ppm/ml: ");
    Serial.println(ppm_per_ml);
    Serial.print("Web post: ");
    Serial.println(webPostEnabled && wifiReady ? "ON" : "OFF");
    Serial.println("====================");
  }
}

void autoRun() {
  if (!isWorkingTime()) return;
  DateTime now = rtc.now();
  int nowMin = now.hour() * 60 + now.minute();
  int cycle = (nowMin - 360) % 40;
  if (cycle < 0) cycle += 40;
  int sec = now.second();
  unsigned long t_ms = (cycle * 60 + sec) * 1000UL;

  if (cycle < 10) {
    if (t_ms < 120000) return;
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck < 30000) return;
    lastCheck = millis();

    float ppm = readTDS();
    Serial.print("PPM: ");
    Serial.println(ppm);
    if (ppm < 550) {
      float deficit = 600 - ppm;
      float ml = (deficit / ppm_per_ml) * 0.3;
      Serial.print("Dose ML: ");
      Serial.println(ml);
      doseEqualML(ml);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ConnectUyenAnh: bom+RTC+calib+WEB (WEB ON) ===");

  pinMode(TDS_PIN, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopAll();

  sensors.begin();
  analogReadResolution(12);

  Wire.begin(SDA_RTC, SCL_RTC);
  if (!rtc.begin()) {
    Serial.println("RTC FAIL");
    while (1);
  }

  // ===== CHỈ MỞ 1 LẦN ĐỂ SET GIỜ RTC =====
  // rtc.adjust(DateTime(2026, 5, 8, 15, 30, 0));

  loadCalib();

  Serial.println("CAL 1000/1382 -> FLOW/ML -> PPM -> WEB ON (sau khi du calib).");
  Serial.println("Mac dinh KHONG WiFi / KHONG gui web.");
  Serial.println("READY");
}

void loop() {
  if (webPostEnabled && wifiReady) {
    if (millis() - lastSync > 21600000UL) {
      Serial.println("Re-sync NTP...");
      configTime(gmtOffset_sec, 0, ntpServer);
      lastSync = millis();
    }

    if (millis() - lastMeasure > 5000) {
      lastMeasure = millis();

      float tdsWeb = readAveragePPMCalibratedForWeb();
      float tempWeb = readTempCalibrated();
      temperature = tempWeb;
      mode = getMode();

      Serial.println("=== POST WEB ===");
      Serial.print("TB 30 ppm (da calib): ");
      Serial.println(tdsWeb);
      Serial.print("Temp (calib): ");
      Serial.println(tempWeb);
      Serial.print("Mode: ");
      Serial.println(mode);

      sendData(tdsWeb, tempWeb);
    }
  }

  handleSerial();
  if (running) autoRun();
  delay(10);
}
