/*
  LED Matrix Running Text für Arduino Uno R4 WiFi
  
  Einfache Funktion um einen String als scrollenden Text anzuzeigen.
  Basiert auf den offiziellen Arduino Beispielen.
*/

// WICHTIG: ArduinoGraphics MUSS vor Arduino_LED_Matrix eingebunden werden
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

ArduinoLEDMatrix matrix;

void setup() {
  Serial.begin(115200);
  matrix.begin();
  
  Serial.println("LED Matrix Text Display gestartet!");
  
  // Teste die Funktion mit einem Dateinamen
  displayScrollingText("DATA_LOG_2025_06_26.TXT");
}

void loop() {
  // Zeige verschiedene Texte
  displayScrollingText("DATA_LOG_2025_06_26.TXT");
  delay(3000);
  
  displayScrollingText("SENSOR_DATA.CSV");
  delay(3000);
  
  displayScrollingText("CONFIG.INI");
  delay(3000);
}

// Hauptfunktion: Zeigt einen String als scrollenden Text an
void displayScrollingText(String text) {
  Serial.println("Zeige Text: " + text);
  
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(60);  // Scroll-Geschwindigkeit
  
  // Verwende Font 5x7 für bessere Lesbarkeit
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println("   " + text + "   ");  // Leerzeichen für besseres Scrolling
  matrix.endText(SCROLL_LEFT);
  
  matrix.endDraw();
}

