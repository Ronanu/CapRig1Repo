#include "FspTimer.h"

FspTimer timer; // Objekt für einen GPT-Timer

volatile uint32_t irqCount = 0;

// Wird bei jedem Timer-Interrupt aufgerufen
void onTimer(timer_callback_args_t* /*args*/) {
  irqCount++;
  digitalWrite(LED_BUILTIN, irqCount & 1);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  // Timer auf GPT, Channel 1 (Timer 1), Frequenz 10 kHz
  bool ok = timer.begin(TIMER_MODE_PERIODIC, GPT_TIMER, 1, 10000.0f, 0.0f, onTimer);
  if (!ok) {
    Serial.println("Timer initialisierung fehlgeschlagen");
    while (true);
  }
}

void loop() {
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();
    Serial.print("Interrupts/s: ");
    Serial.println(irqCount);
    irqCount = 0;
  }
}