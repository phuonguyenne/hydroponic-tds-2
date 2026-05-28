#include <Arduino.h>

#define TDS_PIN 5
#define RAW_CALIB_N 30
#define CAL_SAMPLE_INTERVAL_MS 2000

static float targetPPM = 150.0f;
static float offsetB = 0.0f; // TDS = rawAvg30 + offsetB

float readRawAvg30() {
  float sum = 0.0f;
  for (int i = 0; i < RAW_CALIB_N; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  return sum / (float)RAW_CALIB_N;
}

float runCalibAndPrintSamples() {
  float sum = 0.0f;
  Serial.println("Bat dau lay 30 mau RAW (moi mau cach nhau 2s)...");
  for (int i = 0; i < RAW_CALIB_N; i++) {
    float sample = (float) analogRead(TDS_PIN);
    sum += sample;
    Serial.print("RAW[");
    Serial.print(i + 1);
    Serial.print("] = ");
    Serial.println(sample, 3);

    if (i < RAW_CALIB_N - 1) {
      delay(CAL_SAMPLE_INTERVAL_MS);
    }
  }
  return sum / (float) RAW_CALIB_N;
}

void printHelp() {
  Serial.println("=== CALIB TDS OFFSET (RAW30) ===");
  Serial.println("Lenh:");
  Serial.println("  CAL         -> Tinh offsetB de quy ve targetPPM");
  Serial.println("  TARGET <x>  -> Doi targetPPM (mac dinh 150)");
  Serial.println("  SHOW        -> In RAW30, offsetB, TDS_EST");
}

void setup() {
  Serial.begin(115200);
  pinMode(TDS_PIN, INPUT);
  analogReadResolution(12);
  delay(500);
  printHelp();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("CAL")) {
      float rawRef = runCalibAndPrintSamples();
      offsetB = targetPPM - rawRef;
      Serial.print("RAW_REF_AVG30 = ");
      Serial.println(rawRef, 3);
      Serial.print("OFFSET_B = ");
      Serial.println(offsetB, 3);
      Serial.println("Cong thuc final: TDS = RAW_AVG30 + OFFSET_B");
    } else if (cmd.startsWith("TARGET")) {
      float t = cmd.substring(6).toFloat();
      if (t > 0) {
        targetPPM = t;
      }
      Serial.print("targetPPM = ");
      Serial.println(targetPPM, 2);
    } else if (cmd.equalsIgnoreCase("SHOW")) {
      float raw = readRawAvg30();
      float tds = raw + offsetB;
      if (tds < 0) tds = 0;
      Serial.print("RAW_AVG30 = ");
      Serial.print(raw, 3);
      Serial.print(" | OFFSET_B = ");
      Serial.print(offsetB, 3);
      Serial.print(" | TDS_EST = ");
      Serial.println(tds, 3);
    } else {
      printHelp();
    }
  }

  static unsigned long lastLog = 0;
  if (millis() - lastLog >= 3000) {
    lastLog = millis();
    float raw = readRawAvg30();
    float tds = raw + offsetB;
    if (tds < 0) tds = 0;
    Serial.print("[LOG] RAW_AVG30=");
    Serial.print(raw, 3);
    Serial.print(", OFFSET_B=");
    Serial.print(offsetB, 3);
    Serial.print(", TDS_EST=");
    Serial.println(tds, 3);
  }
}

