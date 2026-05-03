#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= CẤU HÌNH =================
const char* ssid = "Phuong Uyen";
const char* password = "0914859672hong.";

// Production (Railway). Đổi domain nếu Railway cấp URL khác.
const char* kServerBase = "https://hydroponic-tds-2-production.up.railway.app";
String serverInsert = String(kServerBase) + "/insert.php";
String serverMode   = String(kServerBase) + "/get-mode.php";

// NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;   // +7 VN

// CHU KỲ
#define CYCLE_TOTAL 2400
#define ON_TIME 600
#define START_HOUR 6

// PIN
#define TDS_PIN 5
#define ONE_WIRE_BUS 4

#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16

// SENSOR
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// TDS calibration (có thể cần calib lại thực tế)
#define SCOUNT 30
float a = 2.2358;
float b = -3183.67;

// CALIB BƠM
float ml_per_sec_A = 3.0;
float ml_per_sec_B = 2.25;
float ppm_per_ml = 40.0;

// CONTROL
unsigned long lastMeasure = 0;
unsigned long lastPump = 0;
unsigned long lastSync = 0;
#define MIX_TIME 30000

int pumpCount = 0;
String mode = "non";

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n=== HYDROPONIC ESP32 STARTING ===");

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopPump();

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
  Serial.println("NTP configured. Waiting for time sync...");

  // Đợi NTP sync lần đầu (tối đa ~10s)
  struct tm timeinfo;
  for(int i = 0; i < 20; i++) {
    if(getLocalTime(&timeinfo, 1000)) {  // timeout 1s mỗi lần
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

  // ===== LUÔN ĐO & GỬI (không phụ thuộc timeinfo nữa) =====
  if (millis() - lastMeasure > 5000) {
    lastMeasure = millis();

    float tds = readTDS();
    float temp = readTemp();
    mode = getMode();

    Serial.println("=== MEASUREMENT ===");
    Serial.print("TDS: ");  Serial.println(tds);
    Serial.print("Temp: "); Serial.println(temp);
    Serial.print("Mode: "); Serial.println(mode);

    sendData(tds, temp);   // Luôn gửi

    // Kiểm tra cycle để quyết định bơm hay nghỉ
    struct tm timeinfo;
    if (getLocalTime(&timeinfo, 100)) {  // timeout ngắn hơn
      int secondsToday = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;
      int offset = secondsToday - START_HOUR * 3600;

      if (offset >= 0) {
        int cycle = offset % CYCLE_TOTAL;
        if (cycle < 5) pumpCount = 0;   // reset pumpCount đầu chu kỳ

        if (cycle < ON_TIME) {
          Serial.println("=== RUN (TƯỚI) ===");
          controlTDS(tds);
        } else {
          Serial.println("=== REST (NGHỈ) ===");
        }
      } else {
        Serial.println("=== ĐANG NGHỈ BAN ĐÊM (chờ 6h sáng) ===");
      }
    } else {
      Serial.println("Warning: Cannot get local time yet.");
    }
  }

  delay(10); // tránh watchdog
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

// ================= READ TEMP =================
float readTemp() {
  sensors.requestTemperatures();
  float t = sensors.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C) {
    Serial.println("Temp sensor error!");
    return 25.0; // fallback
  }
  return t;
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

// ================= CONTROL TDS =================
void controlTDS(float tds) {
  if (millis() - lastPump < MIX_TIME) return;
  if (pumpCount >= 3) return;

  float minTDS = (mode == "non") ? 500 : 700;
  float maxTDS = (mode == "non") ? 700 : 900;

  if (tds < minTDS) {
    float deficit = minTDS - tds;
    if (deficit > 80) deficit = 80;

    float ml = deficit / ppm_per_ml;

    Serial.print("LOW TDS -> Pumping "); Serial.print(ml); Serial.println(" ml");

    runPumpAB(ml);

    lastPump = millis();
    pumpCount++;
  }
}

// ================= RUN PUMP =================
void runPumpAB(float ml) {
  float tA = (ml / ml_per_sec_A) * 1000;
  float tB = (ml / ml_per_sec_B) * 1000;

  unsigned long start = millis();

  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);   // Pump A
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);   // Pump B

  while (true) {
    unsigned long t = millis() - start;

    if (t >= tA) {
      digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    }
    if (t >= tB) {
      digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
      break;
    }
    delay(1);
  }
  stopPump();
}

void stopPump() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}