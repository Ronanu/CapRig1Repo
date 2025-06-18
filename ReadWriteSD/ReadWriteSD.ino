#include <SD.h>

const int chipSelect = A5;      // Passe ggf. an dein Setup an
const int cardDetectPin = A4;   // Card Detect Pin

File myFile;

void setup() {
  Serial.begin(9600);
  delay(2000);
  pinMode(cardDetectPin, INPUT);

  int status = digitalRead(cardDetectPin);
  if (status == LOW) {
    Serial.println("Keine SD-Karte eingesteckt!");
    while (true);
  } else {
    Serial.println("SD-Karte erkannt. Initialisiere...");
  }

  if (!SD.begin(chipSelect)) {
    Serial.println("SD-Initialisierung fehlgeschlagen!");
    while (true);
  }
  Serial.println("SD-Initialisierung erfolgreich.");

  // Schreibversuch
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Schreibe in test.txt...");
    myFile.println("Hallo von der SD-Karte!");
    myFile.close();
    Serial.println("fertig.");
  } else {
    Serial.println("Fehler beim Öffnen von test.txt zum Schreiben!");
  }

  // Leseversuch
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("Inhalt von test.txt:");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
    Serial.println();
  } else {
    Serial.println("Fehler beim Öffnen von test.txt zum Lesen!");
  }
}

void loop() {
  // nichts passiert nach setup
}