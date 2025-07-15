#include "TimerCallback_ESP32.h"

static uint32_t starttime = 0; // Startzeit für die Messung

// Drei Timer-Objekte
TimerCallback timer10kHz;
TimerCallback timer1kHz;
TimerCallback timer100Hz;

// Drei Counter (volatile für Interrupt-Sicherheit)
volatile uint32_t count10kHz = 0;
volatile uint32_t count1kHz  = 0;
volatile uint32_t count100Hz = 0;

// Drei Callbacks (IRAM_ATTR für bessere Performance bei ESP32)
void IRAM_ATTR callback10kHz(void*) { count10kHz++; }
void IRAM_ATTR callback1kHz(void*)  { count1kHz++; }
void IRAM_ATTR callback100Hz(void*) { count100Hz++; }

// Interrupt-Handler in der .ino-Datei definiert
void IRAM_ATTR onTimer10kHz() {
    timer10kHz.handleInterrupt();
}

void IRAM_ATTR onTimer1kHz() {
    timer1kHz.handleInterrupt();
}

void IRAM_ATTR onTimer100Hz() {
    timer100Hz.handleInterrupt();
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // ESP32 braucht etwas Zeit zum Starten
  
  Serial.println("ESP32 Timer-Test gestartet...");

  bool ok = true;

  // Timer initialisieren mit externen Interrupt-Handlern
  ok &= timer100Hz.begin(100.0f, onTimer100Hz);    // 100 Hz
  ok &= timer1kHz.begin(1000.0f, onTimer1kHz);     // 1 kHz  
  ok &= timer10kHz.begin(10000.0f, onTimer10kHz);  // 10 kHz

  if (!ok) {
    Serial.println("Ein oder mehrere Timer konnten nicht initialisiert werden.");
    while (true) {
      delay(1000);
      Serial.println("Fehler: Timer-Initialisierung fehlgeschlagen!");
    }
  }

  // Callbacks anhängen
  timer10kHz.attachCallback(callback10kHz, nullptr);
  timer1kHz.attachCallback(callback1kHz, nullptr);
  timer100Hz.attachCallback(callback100Hz, nullptr);

  // Timer starten
  ok = timer100Hz.start();
  ok &= timer1kHz.start();
  ok &= timer10kHz.start();

  if (!ok) {
    Serial.println("Ein oder mehrere Timer konnten nicht gestartet werden.");
    while (true) {
      delay(1000);
      Serial.println("Fehler: Timer-Start fehlgeschlagen!");
    }
  }

  Serial.println("Alle Timer gestartet.");
  Serial.println("Erwartete Werte pro Sekunde: 10kHz=10000, 1kHz=1000, 100Hz=100");
  starttime = millis();
}

void loop() {
  static uint32_t lastMillis = 0;

  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();

    // Sicheres Auslesen mit Interrupt-Schutz
    portDISABLE_INTERRUPTS();  // ESP32-spezifisch statt noInterrupts()
    uint32_t c10 = count10kHz;
    uint32_t c1  = count1kHz;
    uint32_t c100 = count100Hz;
    count10kHz = count1kHz = count100Hz = 0;
    portENABLE_INTERRUPTS();   // ESP32-spezifisch statt interrupts()

    // Timer-Steuerung: Nach 5 Sekunden stoppen, nach 10 Sekunden wieder starten
    if (millis() - starttime > 5000 && millis() - starttime <= 10000) {
      static bool stopped = false;
      if (!stopped) {
        Serial.println("*** Timer werden gestoppt (5s erreicht) ***");
        timer10kHz.stop();
        timer1kHz.stop();
        timer100Hz.stop();
        stopped = true;
      }
    }

    if (millis() - starttime > 10000) {
      static bool restarted = false;
      if (!restarted) {
        Serial.println("*** Timer werden neu gestartet (10s erreicht) ***");
        timer10kHz.start();
        timer1kHz.start();
        timer100Hz.start();
        starttime = millis(); // Reset der Startzeit
        restarted = false; // Damit es wieder passieren kann
      }
    }

    // Ausgabe der Messwerte
    Serial.print("Laufzeit: ");
    Serial.print((millis() - starttime) / 1000.0f, 1);
    Serial.print("s | 10kHz: ");
    Serial.print(c10);
    Serial.print(" | 1kHz: ");
    Serial.print(c1);
    Serial.print(" | 100Hz: ");
    Serial.print(c100);
    
    // Performance-Analyse
    float efficiency10k = (c10 / 10000.0f) * 100.0f;
    float efficiency1k = (c1 / 1000.0f) * 100.0f;
    float efficiency100 = (c100 / 100.0f) * 100.0f;
    
    Serial.print(" | Effizienz: ");
    Serial.print(efficiency10k, 1);
    Serial.print("% / ");
    Serial.print(efficiency1k, 1);
    Serial.print("% / ");
    Serial.print(efficiency100, 1);
    Serial.println("%");
  }
  
  // ESP32 kann hier andere Tasks ausführen
  // yield(); ist nicht nötig, da ESP32 ein echtes RTOS verwendet
}
