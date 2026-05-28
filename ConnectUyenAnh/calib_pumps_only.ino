#include <Arduino.h>

#define IN1 6
#define IN2 7
#define IN3 15
#define IN4 16

static float flowA = 0.0f; // ml/s
static float flowB = 0.0f; // ml/s
static int lastSecA = 10;
static int lastSecB = 10;

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
  Serial.println("FLOW A <sec>  / FLOW B <sec>");
  Serial.println("ML A <ml>     / ML B <ml>");
  Serial.println("TEST <ml>     / SHOW");
}

void setup() {
  Serial.begin(115200);
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

  if (cmd.startsWith("FLOW A")) {
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
  } else if (cmd.equalsIgnoreCase("SHOW")) {
    Serial.print("flowA = ");
    Serial.print(flowA, 4);
    Serial.println(" ml/s");
    Serial.print("flowB = ");
    Serial.print(flowB, 4);
    Serial.println(" ml/s");
  } else {
    printHelp();
  }
}

