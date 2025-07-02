#include "sd_card_handler.h"

SDCardHandler sdCard(10, A4);

void writeCsvLine(File& file) {
    float temperatur = 23.5;
    int feuchtigkeit = 42;
    const char* kommentar = "Hallo";
    const char* datum = "2024-06-18";
    file.print(temperatur, 1); file.print(",");
    file.print(feuchtigkeit); file.print(",");
    file.print(kommentar); file.print(",");
    file.println(datum);
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("DEBUG: Start setup()");
  
  // Direkte SD-Library Tests ohne Handler
  Serial.println("=== DIREKTE SD TESTS ===");
  
  // Pin 10 konfigurieren
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  Serial.println("Test 1: SD.begin(10) - Standard");
  if (SD.begin(10)) {
    Serial.println("SUCCESS: Standard funktioniert!");
    testDirectSD();
    return;
  }
  Serial.println("Fehlgeschlagen");
  
  Serial.println("Test 2: SD.begin(10, SPI_HALF_SPEED)");
  if (SD.begin(10, SPI_HALF_SPEED)) {
    Serial.println("SUCCESS: Halbe Geschwindigkeit funktioniert!");
    testDirectSD();
    return;
  }
  Serial.println("Fehlgeschlagen");
  
  Serial.println("Test 3: SD.begin(10, SPI_QUARTER_SPEED)");
  if (SD.begin(10, SPI_QUARTER_SPEED)) {
    Serial.println("SUCCESS: Viertel Geschwindigkeit funktioniert!");
    testDirectSD();
    return;
  }
  Serial.println("Fehlgeschlagen");
  
  Serial.println("ALLE DIREKTEN TESTS FEHLGESCHLAGEN!");
  Serial.println("Mögliche Ursachen:");
  Serial.println("1. SD-Karte defekt/nicht kompatibel");
  Serial.println("2. SD-Karte nicht richtig formatiert (FAT16/FAT32)");
  Serial.println("3. Stromversorgung unzureichend");
  Serial.println("4. SD-Karte zu alt/zu neu");
  Serial.println("5. Kontakte verschmutzt");
}

void testDirectSD() {
  Serial.println("\n=== SD DIREKT-TEST ===");
  
  // Schreibtest
  File file = SD.open("direct.txt", FILE_WRITE);
  if (file) {
    file.println("Direkter Test erfolgreich!");
    file.println("Zeitstempel: 2024-07-02");
    file.close();
    Serial.println("Schreiben: OK");
  } else {
    Serial.println("Schreiben: FEHLER");
    return;
  }
  
  // Lesetest
  file = SD.open("direct.txt", FILE_READ);
  if (file) {
    Serial.println("Dateiinhalt:");
    while (file.available()) {
      Serial.write(file.read());
    }
    file.close();
    Serial.println("Lesen: OK");
  } else {
    Serial.println("Lesen: FEHLER");
  }
  
  Serial.println("\n=== DIREKT-TEST ERFOLGREICH ===");
  Serial.println("SD-Karte funktioniert! Problem liegt am Handler.");
}

void printFileContents(const char* filename) {
    File file = SD.open(filename, FILE_READ);
    if (file) {
        Serial.println("--- DATEIINHALT ---");
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
        Serial.println("\n--- ENDE ---");
    } else {
        Serial.println("FEHLER: Datei konnte nicht geöffnet werden!");
    }
}

void loop() {}