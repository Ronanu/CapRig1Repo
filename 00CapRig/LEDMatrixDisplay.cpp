/*
  LEDMatrixDisplay.cpp
  
  Implementation der LED Matrix Text Display Klasse
  Arduino Uno R4 WiFi
*/

#include "LEDMatrixDisplay.h"

// Konstruktor
LEDMatrixDisplay::LEDMatrixDisplay() {
  scrollSpeed = 60;  // Standard Scroll-Geschwindigkeit
  isInitialized = false;
}

// Destruktor
LEDMatrixDisplay::~LEDMatrixDisplay() {
  // Cleanup falls nötig
}

// Initialisierung der LED Matrix
bool LEDMatrixDisplay::begin() {
  matrix.begin();
  matrix.clear();
  
  isInitialized = true;
  
  return true;
}

// Hauptfunktion: Zeigt einen String als scrollenden Text an
void LEDMatrixDisplay::displayScrollingText(String text) {
  if (!isInitialized) {
    return;
  }
  
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(scrollSpeed);
  
  // Verwende Font 5x7 für bessere Lesbarkeit
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println("   " + text + "   ");  // Leerzeichen für besseres Scrolling
  matrix.endText(SCROLL_LEFT);
  
  matrix.endDraw();
}

// Setzt die Scroll-Geschwindigkeit
void LEDMatrixDisplay::setScrollSpeed(int speed) {
  if (speed > 0) {
    scrollSpeed = speed;
  }
}

// Löscht die Matrix
void LEDMatrixDisplay::clear() {
  if (isInitialized) {
    matrix.clear();
  }
}

// Prüft ob die Matrix bereit ist
bool LEDMatrixDisplay::isReady() {
  return isInitialized;
}
