/*
  00CapRig.ino
  
  Hauptprogramm für das CapRig Projekt
  Arduino Uno R4 WiFi
  
  Verwendet die LEDMatrixDisplay Klasse, ADC78H89 und TimerCallback
*/

#include "LEDMatrixDisplay.h"
#include "ADC78H89.h"
#include "TimerCallback.h"

// Erstelle Objekte
LEDMatrixDisplay ledDisplay;
ADC78H89 adc(0);  // CS-Pin auf D10 (Parameter 0 entspricht D10)
TimerCallback timer100Hz;

// ADC Sampling Variablen
volatile uint16_t adcValue = 0;          // Aktueller ADC-Wert
volatile bool newAdcData = false;        // Flag für neue ADC-Daten
volatile unsigned long sampleCount = 0;  // Anzahl der Samples

// Timer Callback Funktion für 100 Hz ADC Sampling
void adcSamplingCallback(void* context) {
  // ADC-Wert vom ersten Kanal (AIN0) lesen - entspricht dem Beispiel
  adcValue = adc.readChannel(6);  // Kanal 0 = AIN0
  newAdcData = true;
  sampleCount++;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== CapRig Projekt - LED Matrix Display + ADC78H89 Sampling ===");
  Serial.println("Starte Initialisierung...");
  
  // ADC78H89 initialisieren
  adc.begin();
  Serial.println("ADC78H89 initialisiert!");
  
  // Timer für 100 Hz ADC Sampling konfigurieren
  if (timer100Hz.begin(100.0f)) {
    Serial.println("Timer für 100 Hz erfolgreich initialisiert!");
    
    // Timer starten (wie im Beispiel)
    if (timer100Hz.start()) {
      Serial.println("Timer gestartet!");
      
      // Callback-Funktion registrieren (nach dem Start)
      timer100Hz.attachCallback(adcSamplingCallback, nullptr);
      Serial.println("100 Hz ADC Sampling aktiv!");
    } else {
      Serial.println("Fehler beim Starten des 100 Hz Timers!");
    }
  } else {
    Serial.println("Fehler beim Initialisieren des 100 Hz Timers!");
  }
  
  // Initialisiere die LED Matrix
  if (ledDisplay.begin()) {
    Serial.println("LED Matrix erfolgreich initialisiert!");
    Serial.println("Setup erfolgreich abgeschlossen!");
    
    // Zeige erste ADC-Anzeige
    Serial.println("Zeige ADC-Werte auf LED Matrix...");
    ledDisplay.displayScrollingText("AIN0: READY");
  } else {
    Serial.println("Fehler beim Initialisieren der LED Matrix!");
  }
}

void loop() {
  // Prüfe auf neue ADC-Daten
  if (newAdcData) {
    // Interrupt-sichere Kopie erstellen (wie im Timer-Beispiel)
    noInterrupts();
    uint16_t currentAdcValue = adcValue;
    unsigned long currentSampleCount = sampleCount;
    newAdcData = false;
    interrupts();
    
    // ADC-Wert kompakt ausgeben
    if (currentSampleCount % 100 == 0) {  // Alle 100 Samples (jede Sekunde bei 100 Hz)
      Serial.print("AIN0: ");
      Serial.print(currentAdcValue);
      Serial.print(" ");
      Serial.print(currentAdcValue * 5.0 / 4095.0, 2);
      Serial.println(" V");
    }
  }
  
  // LED Matrix Display - zeigt nur ADC-Wert
  static unsigned long lastDisplayUpdate = 0;
  
  if (millis() - lastDisplayUpdate > 1000) { // Jede Sekunde Display aktualisieren
    lastDisplayUpdate = millis();
    
    // Aktuellen ADC-Wert für Display holen
    String adcDisplayText = getAdcDisplayString();
    ledDisplay.displayScrollingText(adcDisplayText);
  }
  
  // Kurze Pause um CPU-Last zu reduzieren
  delay(1);
}

// Hilfsfunktion um ADC-Wert als String für Display zurückzugeben
String getAdcDisplayString() {
  noInterrupts();
  uint16_t currentValue = adcValue;
  interrupts();
  
  float voltage = currentValue * 5.0 / 4095.0;
  return "AIN0: " + String(currentValue) + " (" + String(voltage, 2) + "V)";
}