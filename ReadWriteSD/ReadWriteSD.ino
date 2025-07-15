#include "sd_card_handler.h"

SDCardHandler sdCard(5, 16); // CS auf Pin 16, Card Detect auf Pin 17

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

// Alternative: Teste SD-Karte direkt ohne Card Detect
void testSDCardDirect() {
    Serial.println("DEBUG: Teste SD-Karte direkt ohne Card Detect...");
    if (SD.begin(16)) {
        Serial.println("DEBUG: SD-Karte direkt initialisiert!");
        
        // Teste Schreibvorgang
        File testFile = SD.open("test.txt", FILE_WRITE);
        if (testFile) {
            testFile.println("Test erfolgreich");
            testFile.close();
            Serial.println("DEBUG: Test-Datei geschrieben");
        } else {
            Serial.println("DEBUG: Konnte Test-Datei nicht erstellen");
        }
    } else {
        Serial.println("DEBUG: SD-Karte direkt-Initialisierung fehlgeschlagen");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("DEBUG: Start setup()");
    Serial.println("DEBUG: ESP32 SD-Karten Test");
    Serial.println("DEBUG: CS Pin: 16, Card Detect Pin: 17");
    sdCard.begin();
    Serial.println("DEBUG: SDCardHandler.begin() abgeschlossen");
}

void loop() {
    if (!hasRun) {
        Serial.println("DEBUG: Im loop(), einmaliger Ablauf");

        Serial.println("DEBUG: Prüfe Card Detect Status...");
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
            Serial.println("DEBUG: Versuche SD-Karte direkt...");
            testSDCardDirect();
        }

        hasRun = true;
        lastPrintTime = millis();
    }

    if (hasRun && millis() - lastPrintTime >= 5000) {
        printFileContents("daten.csv");
        lastPrintTime = millis();
    }
}