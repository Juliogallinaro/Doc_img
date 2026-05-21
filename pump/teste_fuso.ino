#include <AccelStepper.h>

// === PINS ===
#define DIR_PIN_1   2
#define STEP_PIN_1  3
#define EN_PIN_1    4

#define DIR_PIN_2   5
#define STEP_PIN_2  6
#define EN_PIN_2    7

AccelStepper stepper1(AccelStepper::DRIVER, STEP_PIN_1, DIR_PIN_1);
AccelStepper stepper2(AccelStepper::DRIVER, STEP_PIN_2, DIR_PIN_2);

// === HARDWARE ===
const float          STEPS_PER_REV = 6400.0;
float                leadMmPerRev  = 1.0;
float                moveMm        = 1.0;
const unsigned long  PAUSE_MS      = 1000;

// === PER-MOTOR STATE ===
struct Motor {
  AccelStepper* stepper;
  uint8_t       enPin;
  bool          running;
  int           dirSign;
  bool          waiting;
  unsigned long waitStart;
};

Motor motors[2] = {
  { &stepper1, EN_PIN_1, false, 1, false, 0 },
  { &stepper2, EN_PIN_2, false, 1, false, 0 }
};

// ============================================================
void applyConfig(Motor& m) {
  float stepsPerMm = STEPS_PER_REV / leadMmPerRev;
  m.stepper->setMaxSpeed(stepsPerMm * 1.0);
  m.stepper->setAcceleration(stepsPerMm * 2.0);
}

void enableMotor(Motor& m) {
  digitalWrite(m.enPin, LOW);
}

void disableMotor(Motor& m) {
  m.running = false;
  m.waiting = false;
  m.stepper->stop();
  digitalWrite(m.enPin, HIGH);
}

void startMotor(Motor& m) {
  m.running   = true;
  m.waiting   = false;
  m.dirSign   = 1;
  m.stepper->move((long)(moveMm * STEPS_PER_REV / leadMmPerRev));
}

void stopMotor(Motor& m) {
  m.running = false;
  m.waiting = false;
  m.stepper->stop();
}

void runMotor(Motor& m) {
  if (!m.running) return;

  if (m.waiting) {
    if (millis() - m.waitStart >= PAUSE_MS) {
      m.waiting = false;
      m.dirSign = -m.dirSign;
      m.stepper->move((long)(moveMm * STEPS_PER_REV / leadMmPerRev) * m.dirSign);
    }
  } else if (m.stepper->distanceToGo() == 0) {
    m.waiting   = true;
    m.waitStart = millis();
  }
}

// ============================================================
void setup() {
  Serial.begin(115200);

  pinMode(EN_PIN_1, OUTPUT); digitalWrite(EN_PIN_1, LOW);
  pinMode(EN_PIN_2, OUTPUT); digitalWrite(EN_PIN_2, LOW);

  for (auto& m : motors) applyConfig(m);

  Serial.print(F("STEPS_PER_REV: ")); Serial.println(STEPS_PER_REV);
  Serial.print(F("Lead (mm/rev): ")); Serial.println(leadMmPerRev, 4);
  Serial.print(F("Move (mm):     ")); Serial.println(moveMm, 3);
  Serial.println(F("-----------------------------"));
  Serial.println(F("Global commands (both motors):"));
  Serial.println(F("  START              -> oscillate both"));
  Serial.println(F("  STOP               -> stop both"));
  Serial.println(F("  ENABLE             -> enable both"));
  Serial.println(F("  DISABLE            -> disable both"));
  Serial.println(F("  LEAD <mm/rev>      -> leadscrew pitch (e.g. LEAD 0.9826)"));
  Serial.println(F("  MOVE <mm>          -> oscillation amplitude (e.g. MOVE 2.5)"));
  Serial.println(F("Per-motor commands (prefix 1 or 2):"));
  Serial.println(F("  1 START  | 2 START"));
  Serial.println(F("  1 STOP   | 2 STOP"));
  Serial.println(F("  1 ENABLE | 2 ENABLE"));
  Serial.println(F("  1 DISABLE| 2 DISABLE"));
  Serial.println(F("  1 CAL <mm>         -> move a set distance"));
}

void loop() {
  for (auto& m : motors) {
    m.stepper->run();
    runMotor(m);
  }

  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  // --- Global commands ---
  if (cmd.equalsIgnoreCase("START")) {
    for (auto& m : motors) startMotor(m);
    Serial.println(F("Both oscillating..."));
    return;
  }

  if (cmd.equalsIgnoreCase("STOP")) {
    for (auto& m : motors) stopMotor(m);
    Serial.println(F("Both stopped."));
    return;
  }

  if (cmd.equalsIgnoreCase("ENABLE")) {
    for (auto& m : motors) enableMotor(m);
    Serial.println(F("Both enabled."));
    return;
  }

  if (cmd.equalsIgnoreCase("DISABLE")) {
    for (auto& m : motors) disableMotor(m);
    Serial.println(F("Both disabled."));
    return;
  }

  if (cmd.startsWith("LEAD")) {
    float v = cmd.substring(5).toFloat();
    if (v > 0) {
      leadMmPerRev = v;
      for (auto& m : motors) applyConfig(m);
      Serial.print(F("Lead -> ")); Serial.print(v, 4);
      Serial.println(F(" mm/rev"));
    }
    return;
  }

  if (cmd.startsWith("MOVE")) {
    float v = cmd.substring(5).toFloat();
    if (v > 0) {
      moveMm = v;
      Serial.print(F("Move -> ")); Serial.print(v, 3);
      Serial.println(F(" mm"));
    }
    return;
  }

  // --- Per-motor commands: "1 CMD" or "2 CMD" ---
  int idx = cmd.charAt(0) - '1';
  if (idx < 0 || idx > 1) {
    Serial.println(F("Use prefix 1 or 2, or a global command without prefix."));
    return;
  }

  Motor& m   = motors[idx];
  String sub = cmd.substring(2);
  sub.trim();

  if (sub.equalsIgnoreCase("START")) {
    startMotor(m);
    Serial.print(F("Motor ")); Serial.print(idx + 1);
    Serial.println(F(" oscillating..."));

  } else if (sub.equalsIgnoreCase("STOP")) {
    stopMotor(m);
    Serial.print(F("Motor ")); Serial.print(idx + 1);
    Serial.println(F(" stopped."));

  } else if (sub.equalsIgnoreCase("ENABLE")) {
    enableMotor(m);
    Serial.print(F("Motor ")); Serial.print(idx + 1);
    Serial.println(F(" enabled."));

  } else if (sub.equalsIgnoreCase("DISABLE")) {
    disableMotor(m);
    Serial.print(F("Motor ")); Serial.print(idx + 1);
    Serial.println(F(" disabled."));

  } else if (sub.startsWith("CAL")) {
    float dist = sub.substring(4).toFloat();
    if (dist != 0) {
      stopMotor(m);
      m.stepper->move((long)(dist * STEPS_PER_REV / leadMmPerRev));
      Serial.print(F("Motor ")); Serial.print(idx + 1);
      Serial.print(F(" -> ")); Serial.print(dist, 3);
      Serial.println(F(" mm"));
    }

  } else {
    Serial.println(F("Unknown command."));
  }
}