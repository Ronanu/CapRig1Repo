/*
  00CapRig.ino
  
  Hauptprogramm für das CapRig Projekt
  Arduino Uno R4 WiFi
  
  Verwendet die LEDMatrixDisplay Klasse aus dem Nachbarordner
*/

#include "LEDMatrixDisplay.h"

// Erstelle ein Objekt der LEDMatrixDisplay Klasse
LEDMatrixDisplay ledDisplay;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== CapRig Projekt - LED Matrix Display ===");
  Serial.println("Starte Initialisierung...");
  
  // Initialisiere die LED Matrix
  if (ledDisplay.begin()) {
    Serial.println("LED Matrix erfolgreich initialisiert!");
    Serial.println("Setup erfolgreich abgeschlossen!");
    
    // Zeige Willkommensnachricht
    Serial.println("Zeige Willkommensnachricht...");
    ledDisplay.displayScrollingText("CAPRIG SYSTEM READY");
  } else {
    Serial.println("Fehler beim Initialisieren der LED Matrix!");
  }
}

void loop() {
  // Zeige verschiedene Systemnachrichten
  Serial.println("Zeige: CAPRIG SYSTEM READY");
  ledDisplay.displayScrollingText("CAPRIG SYSTEM READY");
  delay(3000);
  
  Serial.println("Zeige: DATENERFASSUNG AKTIV");
  ledDisplay.displayScrollingText("DATENERFASSUNG AKTIV");
  delay(3000);
  
  Serial.println("Zeige: SENSOREN OK");
  ledDisplay.displayScrollingText("SENSOREN OK");
  delay(3000);
  
  // Schnellere Geschwindigkeit für Statusmeldungen
  Serial.println("Ändere Geschwindigkeit für Statusmeldung");
  ledDisplay.setScrollSpeed(40);  // Schneller
  Serial.println("Zeige: STATUS: ONLINE");
  ledDisplay.displayScrollingText("STATUS: ONLINE");
  delay(2500);
  
  // Zurück zur normalen Geschwindigkeit
  Serial.println("Zurück zur normalen Geschwindigkeit");
  ledDisplay.setScrollSpeed(60);
  Serial.println("Zeige: PROJEKT HESS4MRA");
  ledDisplay.displayScrollingText("PROJEKT HESS4MRA");
  delay(4000);
  
  Serial.println("=== Zyklus beendet ===");
  delay(1000);
}

// Hilfsfunktion für benutzerdefinierte Nachrichten
void displayCustomMessage(String message) {
  if (ledDisplay.isReady()) {
    Serial.println("Zeige benutzerdefinierte Nachricht: " + message);
    ledDisplay.displayScrollingText(message);
  } else {
    Serial.println("LED Matrix nicht bereit!");
  }
}

// Hilfsfunktion für Systemstatus
void displaySystemStatus() {
  displayCustomMessage("SYSTEM STATUS: OK");
}