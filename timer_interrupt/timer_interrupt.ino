#include "TimerCallback_ESP32.h"

// Timer-Objekte
TimerCallback timer10kHz, timer1kHz, timer100Hz;

// Counter
volatile uint32_t count10kHz = 0, count1kHz = 0, count100Hz = 0;

// Callbacks
void IRAM_ATTR callback10kHz(void*) { count10kHz++; }
void IRAM_ATTR callback1kHz(void*)  { count1kHz++; }
void IRAM_ATTR callback100Hz(void*) { count100Hz++; }

// Interrupt-Handler
void IRAM_ATTR onTimer10kHz() { timer10kHz.handleInterrupt(); }
void IRAM_ATTR onTimer1kHz()  { timer1kHz.handleInterrupt(); }
void IRAM_ATTR onTimer100Hz() { timer100Hz.handleInterrupt(); }

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Timer-Test...");

  // Timer einzeln initialisieren mit Debug-Ausgaben
  Serial.print("Initialisiere 100Hz Timer... ");
  bool ok1 = timer100Hz.begin(100.0f, onTimer100Hz);
  Serial.println(ok1 ? "OK" : "FEHLER");
  
  Serial.print("Initialisiere 1kHz Timer... ");
  bool ok2 = timer1kHz.begin(1000.0f, onTimer1kHz);
  Serial.println(ok2 ? "OK" : "FEHLER");
  
  Serial.print("Initialisiere 10kHz Timer... ");
  bool ok3 = timer10kHz.begin(10000.0f, onTimer10kHz);
  Serial.println(ok3 ? "OK" : "FEHLER");

  if (ok1 && ok2 && ok3) {
    Serial.println("Alle Timer initialisiert!");
    
    // Callbacks anhängen
    Serial.print("Callbacks anhängen... ");
    timer100Hz.attachCallback(callback100Hz, nullptr);
    timer1kHz.attachCallback(callback1kHz, nullptr);
    timer10kHz.attachCallback(callback10kHz, nullptr);
    Serial.println("OK");
    
    // Starten
    Serial.print("Timer starten... ");
    bool start1 = timer100Hz.start();
    bool start2 = timer1kHz.start();
    bool start3 = timer10kHz.start();
    Serial.printf("100Hz:%s 1kHz:%s 10kHz:%s\n", 
                  start1?"OK":"FEHLER", start2?"OK":"FEHLER", start3?"OK":"FEHLER");
    
    if (start1 && start2 && start3) {
      Serial.println("Alle Timer gestartet!");
    } else {
      Serial.println("Start-Fehler!");
      while(1) delay(1000);
    }
  } else {
    Serial.println("Initialisierungs-Fehler!");
    Serial.printf("Details: 100Hz=%s, 1kHz=%s, 10kHz=%s\n", 
                  ok1?"OK":"FEHLER", ok2?"OK":"FEHLER", ok3?"OK":"FEHLER");
    while(1) delay(1000);
  }
}

void loop() {
  static uint32_t lastMillis = 0;
  
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    
    // Sichere Werte lesen
    noInterrupts();
    uint32_t c10 = count10kHz, c1 = count1kHz, c100 = count100Hz;
    count10kHz = count1kHz = count100Hz = 0;
    interrupts();
    
    // Ausgabe
    Serial.printf("10kHz:%lu | 1kHz:%lu | 100Hz:%lu\n", c10, c1, c100);
  }
}
