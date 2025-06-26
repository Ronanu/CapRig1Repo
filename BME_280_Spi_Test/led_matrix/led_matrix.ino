/*
  led_matrix.ino
  
  Hauptprogramm für LED Matrix Text Display
  Arduino Uno R4 WiFi
  
  Verwendet die LEDMatrixDisplay Klasse
*/

#include "LEDMatrixDisplay.h"

// Erstelle ein Objekt der LEDMatrixDisplay Klasse
LEDMatrixDisplay ledDisplay;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== LED Matrix Text Display ===");
  Serial.println("Starte Initialisierung...");
  
  // Initialisiere die LED Matrix
  if (ledDisplay.begin()) {
    Serial.println("LED Matrix erfolgreich initialisiert!");
    Serial.println("Setup erfolgreich abgeschlossen!");
    
    // Teste die Klasse mit einem Dateinamen
    Serial.println("Zeige ersten Text...");
    ledDisplay.displayScrollingText("DATA_LOG_2025_06_26.TXT");
  } else {
    Serial.println("Fehler beim Initialisieren der LED Matrix!");
  }
}

void loop() {
  // Zeige verschiedene Dateinamen mit Serial-Ausgabe
  Serial.println("Zeige: DATA_LOG_2025_06_26.TXT");
  ledDisplay.displayScrollingText("DATA_LOG_2025_06_26.TXT");
  delay(4000);
  
  Serial.println("Zeige: SENSOR_DATA.CSV");
  ledDisplay.displayScrollingText("SENSOR_DATA.CSV");
  delay(4000);
  
  Serial.println("Zeige: CONFIG.INI");
  ledDisplay.displayScrollingText("CONFIG.INI");
  delay(4000);
  
  Serial.println("Zeige: SYSTEM_STATUS.LOG");
  ledDisplay.displayScrollingText("SYSTEM_STATUS.LOG");
  delay(4000);
  
  // Ändere die Geschwindigkeit für Abwechslung
  Serial.println("Ändere Geschwindigkeit auf 30 (schneller)");
  ledDisplay.setScrollSpeed(30);  // Schneller
  Serial.println("Zeige: SCHNELLER TEXT!");
  ledDisplay.displayScrollingText("SCHNELLER TEXT!");
  delay(3000);
  
  Serial.println("Ändere Geschwindigkeit auf 100 (langsamer)");
  ledDisplay.setScrollSpeed(100); // Langsamer
  Serial.println("Zeige: LANGSAMER TEXT...");
  ledDisplay.displayScrollingText("LANGSAMER TEXT...");
  delay(3000);
  
  // Zurück zur normalen Geschwindigkeit
  Serial.println("Zurück zur normalen Geschwindigkeit (60)");
  ledDisplay.setScrollSpeed(60);
}

// Hilfsfunktion um einzelne Texte zu testen
void displaySingleText(String text) {
  if (ledDisplay.isReady()) {
    Serial.println("Zeige Text: " + text);
    ledDisplay.displayScrollingText(text);
    delay(3000);
  } else {
    Serial.println("LED Matrix nicht bereit!");
  }
}

