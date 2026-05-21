#include <AccelStepper.h>

#define DIR_PIN  2
#define STEP_PIN 3
#define EN_PIN   4

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

const float STEPS_PER_REV   = 6400.0;
const float LEAD_MM_PER_REV = 0.9826;
const long  STEPS_PER_MM    = (long)(STEPS_PER_REV / LEAD_MM_PER_REV);

const float MOVE_MM        = 2.0;
const float SPEED          = STEPS_PER_MM * 1.0;
const float ACCEL          = STEPS_PER_MM * 2.0;
const unsigned long PAUSE_MS = 1000;  // pausa entre direções (ms)

bool running       = false;
int  dirSign       = 1;
bool waiting       = false;
unsigned long waitStart = 0;

void setup() {
  Serial.begin(115200);
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);

  stepper.setMaxSpeed(SPEED);
  stepper.setAcceleration(ACCEL);

  Serial.print(F("STEPS_PER_MM: "));
  Serial.println(STEPS_PER_MM);
  Serial.println(F("Comandos: START | STOP | CAL <mm>"));
}

void loop() {
  stepper.run();

  if (running) {
    if (waiting) {
      // aguarda 1s sem bloquear
      if (millis() - waitStart >= PAUSE_MS) {
        waiting = false;
        dirSign = -dirSign;
        stepper.move((long)(MOVE_MM * STEPS_PER_MM) * dirSign);
      }
    }
    else if (stepper.distanceToGo() == 0) {
      // movimento concluído → inicia pausa
      waiting   = true;
      waitStart = millis();
    }
  }

  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd.equalsIgnoreCase("START")) {
      running   = true;
      waiting   = false;
      dirSign   = 1;
      stepper.move((long)(MOVE_MM * STEPS_PER_MM));
      Serial.println(F("Oscilando 1mm..."));
    }
    else if (cmd.equalsIgnoreCase("STOP")) {
      running = false;
      waiting = false;
      stepper.stop();
      Serial.println(F("Parado."));
    }
    else if (cmd.startsWith("CAL")) {
      float dist = cmd.substring(4).toFloat();
      if (dist > 0) {
        running = false;
        waiting = false;
        stepper.stop();
        stepper.move((long)(dist * STEPS_PER_MM));
        Serial.print(F("Movendo "));
        Serial.print(dist);
        Serial.println(F(" mm"));
      }
    }
    else {
      Serial.println(F("Comando desconhecido."));
    }
  }
}