#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

// ================= WIFI =================
const char* ssid = "Ori Coffee";
const char* password = "12345670";

// Production (Railway). Đổi domain nếu Railway cấp URL khác.
const char* kServerBase = "https://hydroponic-tds-2-production.up.railway.app";
String serverName = String(kServerBase) + "/insert.php";

// ================= PIN =================
#define ONE_WIRE_BUS  4
#define TDS_PIN       34

// ================= ADC =================
#define VREF     3.3
#define ADC_RES  4095.0
#define SCOUNT   30

int analogBuffer[SCOUNT];
int analogBufferIndex = 0;

// ================= DS18B20 =================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= CALIB =================
float a = 2.2358;
float b = -3183.67;

float calibrateTemp(float x) {
  return -0.027 * x * x + 2.473 * x - 19.535;
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
}

void loop() {
  static unsigned long analogSampleTime = millis();
  if (millis() - analogSampleTime > 40U) {
    analogSampleTime = millis();
    analogBuffer[analogBufferIndex] = analogRead(TDS_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }

  static unsigned long lastSend = millis();
  if (millis() - lastSend >= 5000) { // gửi mỗi 5s
    lastSend = millis();

    float raw = 0;
    for (int i = 0; i < SCOUNT; i++) raw += analogBuffer[i];
    raw /= SCOUNT;

    float TDS = a * raw + b;

    sensors.requestTemperatures();
    float temp_raw = sensors.getTempCByIndex(0);
    float temp_calib = calibrateTemp(temp_raw);

    float voltage = raw * (VREF / ADC_RES);

    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postData = "temp=" + String(temp_calib) +
                        "&tds=" + String(TDS) +
                        "&voltage=" + String(voltage);

      int httpResponseCode = http.POST(postData);

      Serial.print("HTTP Response: ");
      Serial.println(httpResponseCode);

      http.end();
    }
  }
}