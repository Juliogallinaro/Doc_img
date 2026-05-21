#include <AccelStepper.h>

#define DIR_PIN  2
#define STEP_PIN 3
#define EN_PIN   4

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// === CONFIGURAÇÕES DO HARDWARE ===
const float STEPS_PER_REV    = 6400.0;   // passos por volta (motor + microstepping)
const float LEAD_MM_PER_REV  = 0.9826;      // avanço do fuso (mm por volta) 

// === PARÂMETROS ===
float syringeDiameter_mm = 10.0;    // diâmetro da seringa
float flow_ul_min        = 100.0;   // fluxo em µL/min 
float stepsPerSec        = 0.0;

bool running = false;
int dirSign = 1;

// ============================================================
//  Cálculo da velocidade a partir do DIÂMETRO + FLUXO (µL/min)
// ============================================================
void updateSpeed() {
  // Área da seringa (mm²)
  float radius = syringeDiameter_mm / 2.0;
  float area_mm2 = 3.14159265 * radius * radius;

  if (area_mm2 <= 0 || flow_ul_min <= 0) {
    stepsPerSec = 0;
    return;
  }

  // µL/min → mm³/min (1 µL = 1 mm³)
  float Q_mm3_min = flow_ul_min;

  // Velocidade em mm/min
  float v_mm_min = Q_mm3_min / area_mm2;

  // mm/s
  float v_mm_s = v_mm_min / 60.0;

  // Conversão para steps/s
  stepsPerSec = v_mm_s * (STEPS_PER_REV / LEAD_MM_PER_REV);

  // Ajustes do motor
  float maxSpeed = fabs(stepsPerSec) * 1.2 + 10;
  stepper.setMaxSpeed(maxSpeed);
  stepper.setAcceleration(maxSpeed * 2);
  stepper.setSpeed(stepsPerSec * dirSign);
}

void printInfo() {
  Serial.println(F("=== Syringe Pump Info ==="));
  Serial.print(F("Diameter (mm): "));
  Serial.println(syringeDiameter_mm, 3);
  
  Serial.print(F("Flow (uL/min): "));
  Serial.println(flow_ul_min, 3);

  Serial.print(F("Steps/s: "));
  Serial.println(stepsPerSec, 3);

  Serial.print(F("Direction: "));
  Serial.println(dirSign > 0 ? "FORWARD" : "REVERSE");

  Serial.print(F("Running: "));
  Serial.println(running ? "YES" : "NO");

  Serial.println(F("========================="));
}

void setup() {
  Serial.begin(115200);

  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(1000);

  Serial.println(F("Syringe pump ready."));
  Serial.println(F("Commands:"));
  Serial.println(F("  DIAM <mm>         -> define diâmetro interno"));
  Serial.println(F("  FLOW <uL/min>     -> define fluxo em microlitros/min"));
  Serial.println(F("  START             -> iniciar bomba"));
  Serial.println(F("  STOP              -> parar bomba"));
  Serial.println(F("  DIR 1 / DIR -1    -> sentido"));
  Serial.println(F("  INFO              -> status"));
}

void loop() {
  if (running && stepsPerSec != 0) {
    stepper.runSpeed();
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.startsWith("DIAM")) {
      float d = cmd.substring(5).toFloat();
      if (d > 0) {
        syringeDiameter_mm = d;
        updateSpeed();
        Serial.print(F("Diameter set to "));
        Serial.println(d, 3);
      }
    }

    else if (cmd.startsWith("FLOW")) {
      float f = cmd.substring(5).toFloat();
      if (f > 0) {
        flow_ul_min = f;
        updateSpeed();
        Serial.print(F("Flow set to "));
        Serial.print(f, 3);
        Serial.println(F(" uL/min"));
      }
    }

    else if (cmd.equalsIgnoreCase("START")) {
      if (stepsPerSec == 0) {
        Serial.println(F("Configure DIAM e FLOW antes de iniciar."));
      } else {
        running = true;
        stepper.setSpeed(stepsPerSec * dirSign);
        Serial.println(F("Pumping started."));
      }
    }

    else if (cmd.equalsIgnoreCase("STOP")) {
      running = false;
      stepper.setSpeed(0);
      Serial.println(F("Pumping stopped."));
    }

    else if (cmd.startsWith("DIR")) {
      int d = cmd.substring(4).toInt();
      if (d == 1 || d == -1) {
        dirSign = d;
        stepper.setSpeed(stepsPerSec * dirSign);
        Serial.print(F("Direction set to "));
        Serial.println(dirSign > 0 ? "FORWARD" : "REVERSE");
      }
    }

    else if (cmd.equalsIgnoreCase("INFO")) {
      printInfo();
    }

    else {
      Serial.println(F("Unknown command."));
    }
  }
}
