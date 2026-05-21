#include <AccelStepper.h>

// === PINOS ===
#define DIR_PIN_1   2
#define STEP_PIN_1  3
#define EN_PIN_1    4

#define DIR_PIN_2   5
#define STEP_PIN_2  6
#define EN_PIN_2    7

AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);

// === CONFIGURAÇÕES DO HARDWARE ===
const float STEPS_PER_REV = 6400.0;

// === ESTADO DE CADA MOTOR ===
struct Pump {
  AccelStepper* stepper;
  uint8_t       enPin;
  float         diameter_mm;
  float         flow_ul_min;
  float         lead_mm_per_rev;
  float         stepsPerSec;
  bool          running;
  bool          enabled;
  int           dirSign;
};

Pump pumps[2] = {
  { &stepper1, EN_PIN_1, 10.0, 100.0, 0.9950, 0.0, false, true,  1 },
  { &stepper2, EN_PIN_2, 10.0, 100.0, 0.9950, 0.0, false, true,  1 }
};

// ============================================================
void setEnable(Pump& p, bool en) {
  p.enabled = en;
  // Driver A4988/DRV8825: LOW = habilitado, HIGH = desabilitado
  digitalWrite(p.enPin, en ? LOW : HIGH);
  if (!en) {
    p.running = false;
    p.stepper->setSpeed(0);
  }
}

void updateSpeed(Pump& p) {
  float radius   = p.diameter_mm / 2.0;
  float area_mm2 = 3.14159265f * radius * radius;

  if (area_mm2 <= 0 || p.flow_ul_min <= 0 || p.lead_mm_per_rev <= 0) {
    p.stepsPerSec = 0;
    return;
  }

  float v_mm_s  = (p.flow_ul_min / 60.0f) / area_mm2;
  p.stepsPerSec = v_mm_s * (STEPS_PER_REV / p.lead_mm_per_rev);

  float maxSpeed = fabs(p.stepsPerSec) * 1.2f + 10.0f;
  p.stepper->setMaxSpeed(maxSpeed);
  p.stepper->setAcceleration(maxSpeed * 2.0f);
  p.stepper->setSpeed(p.stepsPerSec * p.dirSign);
}

// --- Comandos individuais ---
void doStart(int idx) {
  Pump& p = pumps[idx];
  if (!p.enabled) {
    Serial.print(F("Pump ")); Serial.print(idx + 1);
    Serial.println(F(": desabilitado. Use EN primeiro."));
  } else if (p.stepsPerSec == 0) {
    Serial.print(F("Pump ")); Serial.print(idx + 1);
    Serial.println(F(": configure DIAM e FLOW antes."));
  } else {
    p.running = true;
    p.stepper->setSpeed(p.stepsPerSec * p.dirSign);
    Serial.print(F("Pump ")); Serial.print(idx + 1);
    Serial.println(F(" started."));
  }
}

void doStop(int idx) {
  Pump& p = pumps[idx];
  p.running = false;
  p.stepper->setSpeed(0);
  Serial.print(F("Pump ")); Serial.print(idx + 1);
  Serial.println(F(" stopped."));
}

void printInfo(int idx) {
  Pump& p = pumps[idx];
  Serial.print(F("=== Pump ")); Serial.print(idx + 1); Serial.println(F(" ==="));
  Serial.print(F("Enabled:       ")); Serial.println(p.enabled  ? "YES" : "NO");
  Serial.print(F("Running:       ")); Serial.println(p.running  ? "YES" : "NO");
  Serial.print(F("Diameter (mm): ")); Serial.println(p.diameter_mm,    3);
  Serial.print(F("Flow (uL/min): ")); Serial.println(p.flow_ul_min,    3);
  Serial.print(F("Lead (mm/rev): ")); Serial.println(p.lead_mm_per_rev,4);
  Serial.print(F("Steps/s:       ")); Serial.println(p.stepsPerSec,    3);
  Serial.print(F("Direction:     ")); Serial.println(p.dirSign > 0 ? "FORWARD" : "REVERSE");
}

// ============================================================
void setup() {
  Serial.begin(115200);

  for (auto& p : pumps) {
    pinMode(p.enPin, OUTPUT);
    setEnable(p, true);
    p.stepper->setMaxSpeed(2000);
    p.stepper->setAcceleration(1000);
  }

  Serial.println(F("Dual syringe pump ready."));
  Serial.println(F("--- Comandos individuais (prefixo 1 ou 2) ---"));
  Serial.println(F("  1 DIAM <mm>        2 DIAM <mm>"));
  Serial.println(F("  1 FLOW <uL/min>    2 FLOW <uL/min>"));
  Serial.println(F("  1 LEAD <mm/rev>    2 LEAD <mm/rev>"));
  Serial.println(F("  1 DIR 1/-1         2 DIR 1/-1"));
  Serial.println(F("  1 START            2 START"));
  Serial.println(F("  1 STOP             2 STOP"));
  Serial.println(F("  1 EN               2 EN"));
  Serial.println(F("  1 DIS              2 DIS"));
  Serial.println(F("  1 INFO             2 INFO"));
  Serial.println(F("--- Comandos globais (prefixo ALL) ---"));
  Serial.println(F("  ALL START"));
  Serial.println(F("  ALL STOP"));
  Serial.println(F("  ALL EN"));
  Serial.println(F("  ALL DIS"));
  Serial.println(F("  ALL INFO"));
}

void loop() {
  for (auto& p : pumps) {
    if (p.enabled && p.running && p.stepsPerSec != 0)
      p.stepper->runSpeed();
  }

  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  // --- Comandos globais: ALL <CMD> ---
  if (cmd.length() >= 3 &&
      (cmd.substring(0, 3).equalsIgnoreCase("ALL"))) {
    String sub = cmd.substring(4);
    sub.trim();

    if (sub.equalsIgnoreCase("START")) {
      for (int i = 0; i < 2; i++) doStart(i);

    } else if (sub.equalsIgnoreCase("STOP")) {
      for (int i = 0; i < 2; i++) doStop(i);

    } else if (sub.equalsIgnoreCase("EN")) {
      for (int i = 0; i < 2; i++) {
        setEnable(pumps[i], true);
        Serial.print(F("Pump ")); Serial.print(i + 1);
        Serial.println(F(" enabled."));
      }

    } else if (sub.equalsIgnoreCase("DIS")) {
      for (int i = 0; i < 2; i++) {
        setEnable(pumps[i], false);
        Serial.print(F("Pump ")); Serial.print(i + 1);
        Serial.println(F(" disabled."));
      }

    } else if (sub.equalsIgnoreCase("INFO")) {
      for (int i = 0; i < 2; i++) printInfo(i);

    } else {
      Serial.println(F("ALL: comando desconhecido."));
    }
    return;
  }

  // --- Comandos individuais: 1 ou 2 ---
  int idx = cmd.charAt(0) - '1';
  if (idx < 0 || idx > 1) {
    Serial.println(F("Use prefixo 1, 2 ou ALL."));
    return;
  }

  Pump&  p   = pumps[idx];
  String sub = cmd.substring(2);
  sub.trim();

  if (sub.startsWith("DIAM")) {
    float d = sub.substring(5).toFloat();
    if (d > 0) {
      p.diameter_mm = d;
      updateSpeed(p);
      Serial.print(F("Pump ")); Serial.print(idx + 1);
      Serial.print(F(" diameter -> ")); Serial.print(d, 3);
      Serial.println(F(" mm"));
    }

  } else if (sub.startsWith("FLOW")) {
    float f = sub.substring(5).toFloat();
    if (f > 0) {
      p.flow_ul_min = f;
      updateSpeed(p);
      Serial.print(F("Pump ")); Serial.print(idx + 1);
      Serial.print(F(" flow -> ")); Serial.print(f, 3);
      Serial.println(F(" uL/min"));
    }

  } else if (sub.startsWith("LEAD")) {
    float l = sub.substring(5).toFloat();
    if (l > 0) {
      p.lead_mm_per_rev = l;
      updateSpeed(p);
      Serial.print(F("Pump ")); Serial.print(idx + 1);
      Serial.print(F(" lead -> ")); Serial.print(l, 4);
      Serial.println(F(" mm/rev"));
    }

  } else if (sub.equalsIgnoreCase("START")) {
    doStart(idx);

  } else if (sub.equalsIgnoreCase("STOP")) {
    doStop(idx);

  } else if (sub.equalsIgnoreCase("EN")) {
    setEnable(p, true);
    Serial.print(F("Pump ")); Serial.print(idx + 1);
    Serial.println(F(" enabled."));

  } else if (sub.equalsIgnoreCase("DIS")) {
    setEnable(p, false);
    Serial.print(F("Pump ")); Serial.print(idx + 1);
    Serial.println(F(" disabled."));

  } else if (sub.startsWith("DIR")) {
    int d = sub.substring(4).toInt();
    if (d == 1 || d == -1) {
      p.dirSign = d;
      p.stepper->setSpeed(p.stepsPerSec * p.dirSign);
      Serial.print(F("Pump ")); Serial.print(idx + 1);
      Serial.print(F(" direction -> "));
      Serial.println(d > 0 ? "FORWARD" : "REVERSE");
    }

  } else if (sub.equalsIgnoreCase("INFO")) {
    printInfo(idx);

  } else {
    Serial.println(F("Unknown command."));
  }
}
