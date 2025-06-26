/*
  LEDMatrixDisplay.h
  
  Header file für LED Matrix Text Display Klasse
  Arduino Uno R4 WiFi
*/

#ifndef LEDMATRIXDISPLAY_H
#define LEDMATRIXDISPLAY_H

#include "Arduino.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

class LEDMatrixDisplay {
private:
  ArduinoLEDMatrix matrix;
  int scrollSpeed;
  bool isInitialized;

public:
  // Konstruktor
  LEDMatrixDisplay();
  
  // Initialisierung
  bool begin();
  
  // Hauptfunktion: Zeigt scrollenden Text an
  void displayScrollingText(String text);
  
  // Hilfsfunktionen
  void setScrollSpeed(int speed);
  void clear();
  bool isReady();
  
  // Destruktor
  ~LEDMatrixDisplay();
};

#endif
