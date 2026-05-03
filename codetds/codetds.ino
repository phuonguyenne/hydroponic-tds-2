#include <OneWire.h>
#include <DallasTemperature.h>

// ====================== PIN ======================
#define ONE_WIRE_BUS  5
#define TDS_PIN       4

// ====================== ADC ======================
#define VREF     3.3
#define ADC_RES  4095.0
#define SCOUNT   30

int analogBuffer[SCOUNT];
int analogBufferIndex = 0;

// ====================== DS18B20 ======================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ====================== HỆ SỐ CALIB TDS ======================
float a = 2.2358;
float b = -3183.67;

// ====================== HÀM CALIB NHIỆT ======================
float calibrateTemp(float x) {
  return -0.027 * x * x + 2.473 * x - 19.535;
}

void setup() {
  Serial.begin(115200);
  sensors.begin();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  Serial.println("=== TDS + TEMP (CALIB OK) ===");
}

void loop() {
  // ===== LẤY MẪU TDS LIÊN TỤC =====
  static unsigned long analogSampleTime = millis();
  if (millis() - analogSampleTime > 40U) {
    analogSampleTime = millis();
    analogBuffer[analogBufferIndex] = analogRead(TDS_PIN);
    analogBufferIndex = (analogBufferIndex + 1) % SCOUNT;
  }

  // ===== IN KẾT QUẢ MỖI 2 GIÂY =====
  static unsigned long lastPrint = millis();
  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();

    // ---- TÍNH RAW TRUNG BÌNH ----
    float raw = 0;
    for (int i = 0; i < SCOUNT; i++) {
      raw += analogBuffer[i];
    }
    raw = raw / SCOUNT;

    // ---- TÍNH TDS ----
    float TDS = a * raw + b;

    // ===== NHIỆT ĐỘ =====
    sensors.requestTemperatures();
    float temp_raw = sensors.getTempCByIndex(0);
    float temp_calib = calibrateTemp(temp_raw);

    // ===== IN RA =====
    Serial.print("Temp calib: ");
    Serial.print(temp_calib, 2);
    Serial.print(" °C | ");

    Serial.print("RAW: ");
    Serial.print(raw);

    Serial.print(" | TDS: ");
    Serial.print(TDS, 1);
    Serial.println(" ppm");

    Serial.println("-----------------------------");
  }
}