#include "TimerCallback.h"

TimerCallback myTimer;

volatile uint32_t counter = 0;

void mySensorCallback(void* /*context*/) {
  // KEIN Serial.print() hier – nur zählen
  counter++;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!myTimer.begin(10000.0f)) {
    Serial.println("Timerstart fehlgeschlagen");
    while (true);
  }

  myTimer.attachCallback(mySensorCallback, nullptr);
  Serial.println("Timer läuft, Callback zählt...");
}

void loop() {
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();

    noInterrupts();
    uint32_t copy = counter;
    counter = 0;
    interrupts();

    Serial.print("Sensor-Callbacks pro Sekunde: ");
    Serial.println(copy);
  }
}