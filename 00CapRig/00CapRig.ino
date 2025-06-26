/*
  00CapRig.ino
  
  Hauptprogramm für das CapRig Projekt
  Arduino Uno R4 WiFi
  
  Verwendet die LEDMatrixDisplay Klasse, ADC78H89 und TimerCallback
*/

// ============== INCLUDES ==============
#include "LEDMatrixDisplay.h"
#include "ADC78H89.h"
#include "TimerCallback.h"

// ============== OBJEKTE ==============
LEDMatrixDisplay ledDisplay;
ADC78H89 adc(0);  // CS-Pin auf D10 (Parameter 0 entspricht D10)
TimerCallback timer100Hz;

// ============== VARIABLEN ==============
volatile uint16_t adcValue = 0;          // Aktueller ADC-Wert
volatile bool newAdcData = false;        // Flag für neue ADC-Daten
volatile unsigned long sampleCount = 0;  // Anzahl der Samples

// ============== CALLBACK ==============
void adcSamplingCallback(void* context) {
  adcValue = adc.readChannel(6);  // Kanal 6
  newAdcData = true;
  sampleCount++;
}

// ============== SETUP ==============
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== CapRig Projekt - LED Matrix Display + ADC78H89 Sampling ===");
  Serial.println("Starte Initialisierung...");
  
  // -- ADC initialisieren --
  adc.begin();
  Serial.println("ADC78H89 initialisiert!");
  
  // -- Timer konfigurieren --
  if (timer100Hz.begin(100.0f)) {
    Serial.println("Timer für 100 Hz erfolgreich initialisiert!");
    
    if (timer100Hz.start()) {
      Serial.println("Timer gestartet!");
      timer100Hz.attachCallback(adcSamplingCallback, nullptr);
      Serial.println("100 Hz ADC Sampling aktiv!");
    } else {
      Serial.println("Fehler beim Starten des 100 Hz Timers!");
    }
  } else {
    Serial.println("Fehler beim Initialisieren des 100 Hz Timers!");
  }
  
  // -- LED Matrix initialisieren --
  if (ledDisplay.begin()) {
    Serial.println("LED Matrix erfolgreich initialisiert!");
    Serial.println("Setup erfolgreich abgeschlossen!");
    ledDisplay.displayScrollingText("AIN6: READY");
  } else {
    Serial.println("Fehler beim Initialisieren der LED Matrix!");
  }
}

// ============== MAIN LOOP ==============
void loop() {
  // -- ADC Daten verarbeiten --
  if (newAdcData) {
    noInterrupts();
    uint16_t currentAdcValue = adcValue;
    unsigned long currentSampleCount = sampleCount;
    newAdcData = false;
    interrupts();
    
    // Serial Ausgabe (jede Sekunde)
    if (currentSampleCount % 100 == 0) {
      Serial.print("AIN6: ");
      Serial.print(currentAdcValue);
      Serial.print(" ");
      Serial.print(currentAdcValue * 5.0 / 4095.0, 2);
      Serial.println(" V");
    }
  }
  
  // -- LED Matrix Update --
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 1000) {
    lastDisplayUpdate = millis();
    String adcDisplayText = getAdcDisplayString();
    ledDisplay.displayScrollingText(adcDisplayText);
  }
  
  delay(1);
}

// ============== HELPER FUNCTIONS ==============
String getAdcDisplayString() {
  noInterrupts();
  uint16_t currentValue = adcValue;
  interrupts();
  
  float voltage = currentValue * 5.0 / 4095.0;
  return "AIN6: " + String(currentValue) + " (" + String(voltage, 2) + "V)";
}