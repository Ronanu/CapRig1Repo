#include "TimerCallback.h"

 static uint32_t starttime = 0; // Startzeit für die Messung

// Drei Timer-Objekte
TimerCallback timer10kHz;
TimerCallback timer1kHz;
TimerCallback timer100Hz;

// Drei Counter
volatile uint32_t count10kHz = 0;
volatile uint32_t count1kHz  = 0;
volatile uint32_t count100Hz = 0;

// Drei Callbacks
void callback10kHz(void*) { count10kHz++; }
void callback1kHz(void*)  { count1kHz++; }
void callback100Hz(void*) { count100Hz++; }

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  bool ok = true;

  ok &= timer10kHz.begin(10000.0f);  // 10 kHz
  ok &= timer1kHz.begin(1000.0f);    // 1 kHz
  ok &= timer100Hz.begin(100.0f);    // 100 Hz

  if (!ok) {
    Serial.println("Ein oder mehrere Timer konnten nicht initialisiert werden.");
    while (true);
  }

  ok = timer10kHz.start();
  ok &= timer1kHz.start();
  ok &= timer100Hz.start();

  if (!ok) {
    Serial.println("Ein oder mehrere Timer konnten nicht gestartet werden.");
    while (true);
  }

  timer10kHz.attachCallback(callback10kHz, nullptr);
  timer1kHz.attachCallback(callback1kHz, nullptr);
  timer100Hz.attachCallback(callback100Hz, nullptr);

  Serial.println("Alle Timer gestartet.");
  starttime = millis();
}

void loop() {
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();

    // Sicheres Auslesen mit Interrupt-Schutz
    noInterrupts();
    uint32_t c10 = count10kHz;
    uint32_t c1  = count1kHz;
    uint32_t c100 = count100Hz;
    count10kHz = count1kHz = count100Hz = 0;

    if (millis() - starttime > 5000) { // Nach 5 Sekunden stoppen
      timer10kHz.stop();
      timer1kHz.stop();
      timer100Hz.stop();
    }

    if (millis() - starttime > 10000) { // Nach 10 Sekunden wieder starten
      timer10kHz.start();
      timer1kHz.start();
      timer100Hz.start();
      starttime = millis(); // Reset der Startzeit
    }

    interrupts();

    Serial.print("10kHz-Callbacks/s: ");
    Serial.print(c10);
    Serial.print(" | 1kHz: ");
    Serial.print(c1);
    Serial.print(" | 100Hz: ");
    Serial.println(c100);
  }
}