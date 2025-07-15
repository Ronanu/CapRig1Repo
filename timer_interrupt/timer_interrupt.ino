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

  // Timer initialisieren
  if (timer100Hz.begin(100.0f, onTimer100Hz) && 
      timer1kHz.begin(1000.0f, onTimer1kHz) && 
      timer10kHz.begin(10000.0f, onTimer10kHz)) {
    
    // Callbacks anhängen
    timer100Hz.attachCallback(callback100Hz);
    timer1kHz.attachCallback(callback1kHz);
    timer10kHz.attachCallback(callback10kHz);
    
    // Timer starten
    if (timer100Hz.start() && timer1kHz.start() && timer10kHz.start()) {
      Serial.println("Alle Timer gestartet!");
    } else {
      Serial.println("Start-Fehler!");
      while(1);
    }
  } else {
    Serial.println("Init-Fehler!");
    while(1);
  }
}

void loop() {
  static uint32_t lastMillis = 0;
  
  if (millis() - lastMillis >= 1000) {
    lastMillis = millis();
    
    // Counts lesen und zurücksetzen
    noInterrupts();
    uint32_t c10 = count10kHz, c1 = count1kHz, c100 = count100Hz;
    count10kHz = count1kHz = count100Hz = 0;
    interrupts();
    
    Serial.printf("10kHz:%lu | 1kHz:%lu | 100Hz:%lu\n", c10, c1, c100);
  }
}
