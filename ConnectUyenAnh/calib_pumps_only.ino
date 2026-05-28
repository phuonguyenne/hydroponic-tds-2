#include <Arduino.h>

#define TDS_PIN 5
#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16

static float flowA = 0.0f; // ml/s
static float flowB = 0.0f; // ml/s
static int lastSecA = 10;
static int lastSecB = 10;
static float offsetB = 0.0f;      // copy tu calib_tds_offset_30.ino
static float ppm0 = 0.0f;
static float ppm_per_ml = 0.0f;   // ket qua can lay
static float lastDoseMl = 10.0f;  // ml tong (A+B) cho PPM DOSE

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

float readRawAvg30() {
  float sum = 0.0f;
  for (int i = 0; i < 30; i++) {
    sum += analogRead(TDS_PIN);
    delay(10);
  }
  return sum / 30.0f;
}

float readTDSWithOffset() {
  float raw = readRawAvg30();
  float tds = raw + offsetB;
  if (tds < 0 || raw < 100) tds = 0;
  return tds;
}

void doseEqualML(float ml) {
  if (flowA <= 0 || flowB <= 0) {
    Serial.println("Flow A/B chua duoc calib.");
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
    unsigned long dt = millis() - start;
    if (dt >= (unsigned long) (timeA * 1000.0f)) pumpA_off();
    if (dt >= (unsigned long) (timeB * 1000.0f)) pumpB_off();
    if (dt >= (unsigned long) (max(timeA, timeB) * 1000.0f)) break;
    delay(5);
  }
  stopAll();
}

void printHelp() {
  Serial.println("=== CALIB PUMPS ONLY ===");
  Serial.println("SET OFFSET <b>      // dan offsetB tu calib_tds_offset_30");
  Serial.println("FLOW A <sec>  / FLOW B <sec>");
  Serial.println("ML A <ml>     / ML B <ml>");
  Serial.println("PPM START            // luu ppm0 hien tai");
  Serial.println("PPM DOSE <ml>        // bom tong ml roi luu lastDoseMl");
  Serial.println("PPM END              // tinh ppm_per_ml = (ppm1-ppm0)/lastDoseMl");
  Serial.println("TEST <ml>     / SHOW");
}

void setup() {
  Serial.begin(115200);
  pinMode(TDS_PIN, INPUT);
  analogReadResolution(12);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  stopAll();
  delay(500);
  printHelp();
}

void loop() {
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("SET OFFSET")) {
    float b = cmd.substring(10).toFloat();
    offsetB = b;
    Serial.print("offsetB = ");
    Serial.println(offsetB, 4);
  } else if (cmd.startsWith("FLOW A")) {
    int sec = cmd.substring(7).toInt();
    if (sec <= 0) sec = 10;
    lastSecA = sec;
    Serial.print("Bom A chay ");
    Serial.print(sec);
    Serial.println(" giay...");
    pumpA_on();
    delay(sec * 1000);
    pumpA_off();
    Serial.println("Nhap: ML A <so_ml_do_duoc>");
  } else if (cmd.startsWith("ML A")) {
    float ml = cmd.substring(5).toFloat();
    if (ml > 0 && lastSecA > 0) flowA = ml / (float) lastSecA;
    Serial.print("flowA = ");
    Serial.print(flowA, 4);
    Serial.println(" ml/s");
  } else if (cmd.startsWith("FLOW B")) {
    int sec = cmd.substring(7).toInt();
    if (sec <= 0) sec = 10;
    lastSecB = sec;
    Serial.print("Bom B chay ");
    Serial.print(sec);
    Serial.println(" giay...");
    pumpB_on();
    delay(sec * 1000);
    pumpB_off();
    Serial.println("Nhap: ML B <so_ml_do_duoc>");
  } else if (cmd.startsWith("ML B")) {
    float ml = cmd.substring(5).toFloat();
    if (ml > 0 && lastSecB > 0) flowB = ml / (float) lastSecB;
    Serial.print("flowB = ");
    Serial.print(flowB, 4);
    Serial.println(" ml/s");
  } else if (cmd.startsWith("TEST")) {
    float ml = cmd.substring(4).toFloat();
    if (ml <= 0) ml = 5;
    Serial.print("TEST dose ");
    Serial.print(ml);
    Serial.println(" ml");
    doseEqualML(ml);
    Serial.println("Done");
  } else if (cmd.equalsIgnoreCase("PPM START")) {
    ppm0 = readTDSWithOffset();
    Serial.print("PPM0 = ");
    Serial.println(ppm0, 3);
    Serial.println("Goi PPM DOSE <ml> de bom.");
  } else if (cmd.startsWith("PPM DOSE")) {
    float ml = cmd.substring(8).toFloat();
    if (ml <= 0) ml = 10.0f;
    lastDoseMl = ml;
    Serial.print("PPM DOSE ");
    Serial.print(lastDoseMl, 2);
    Serial.println(" ml");
    doseEqualML(lastDoseMl);
    Serial.println("Da bom xong. Cho tron 60-120s roi go PPM END.");
  } else if (cmd.equalsIgnoreCase("PPM END")) {
    float ppm1 = readTDSWithOffset();
    Serial.print("PPM1 = ");
    Serial.println(ppm1, 3);
    if (lastDoseMl > 0.001f) {
      ppm_per_ml = (ppm1 - ppm0) / lastDoseMl;
    }
    Serial.print("ppm_per_ml = ");
    Serial.println(ppm_per_ml, 5);
  } else if (cmd.equalsIgnoreCase("SHOW")) {
    float tds = readTDSWithOffset();
    Serial.print("flowA = ");
    Serial.print(flowA, 4);
    Serial.println(" ml/s");
    Serial.print("flowB = ");
    Serial.print(flowB, 4);
    Serial.println(" ml/s");
    Serial.print("offsetB = ");
    Serial.println(offsetB, 4);
    Serial.print("tds_now = ");
    Serial.println(tds, 3);
    Serial.print("ppm_per_ml = ");
    Serial.println(ppm_per_ml, 5);
  } else {
    printHelp();
  }
}

