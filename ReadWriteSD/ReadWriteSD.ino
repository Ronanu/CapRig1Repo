

#include "sd_card_handler.h"

SDCardHandler sdCard(16, 17); // CS auf Pin 16, Card Detect auf Pin 17

bool hasRun = false;
unsigned long lastPrintTime = 0;

void writeCsvLine(File& file) {
    float temperatur = 23.5;
    int feuchtigkeit = 42;
    
    // Erstelle CSV-Zeile mit Timestamp, Temperatur und Feuchtigkeit
    String datum = String(millis());
    file.print(datum);
    file.print(",");
    file.print(temperatur);
    file.print(",");
    file.println(feuchtigkeit);
}

void printFileContents(const char* filename) {
    File file = SD.open(filename, FILE_READ);
    if (file) {
        Serial.println("--- Dateiinhalt ---");
        while (file.available()) {
            Serial.write(file.read());
        }
        file.close();
        Serial.println("--- Ende ---");
    } else {
        Serial.println("Fehler beim Öffnen der Datei!");
    }
}

void setup() {
    Serial.begin(9600);
    delay(2000);
    Serial.println("DEBUG: Start setup()");
    sdCard.begin();
}

void loop() {
    if (!hasRun) {
        Serial.println("DEBUG: Im loop(), einmaliger Ablauf");

        if (sdCard.isCardInserted()) {
            Serial.println("DEBUG: Karte erkannt");
            if (sdCard.init()) {
                Serial.println("DEBUG: SD-Karte initialisiert");
                
                // Schreibe Header falls Datei neu erstellt wird
                File testFile = SD.open("daten.csv", FILE_READ);
                if (!testFile) {
                    // Datei existiert nicht, schreibe CSV-Header
                    sdCard.writeStringLine("daten.csv", "Timestamp,Temperatur,Feuchtigkeit");
                    Serial.println("DEBUG: CSV-Header geschrieben");
                } else {
                    testFile.close();
                }
                
                // Schreibe Datenzeile
                if (sdCard.writeCustomLine("daten.csv", writeCsvLine)) {
                    Serial.println("DEBUG: Daten erfolgreich geschrieben");
                } else {
                    Serial.println("DEBUG: Fehler beim Schreiben der Daten");
                }
            } else {
                Serial.println("DEBUG: SD-Karte Initialisierung fehlgeschlagen");
            }
        } else {
            Serial.println("DEBUG: Keine Karte erkannt");
        }

        hasRun = true;
        lastPrintTime = millis();
    }

    if (hasRun && millis() - lastPrintTime >= 5000) {
        printFileContents("daten.csv");
        lastPrintTime = millis();
    }
}