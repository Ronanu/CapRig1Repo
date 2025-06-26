#include "sd_card_handler.h"

SDCardHandler sdCard(A5, A4);

bool hasRun = false;
unsigned long lastPrintTime = 0;

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
                Serial.println("DEBUG: SD init OK");
                sdCard.writeStringLine("daten.csv", "A,B,C,D");
                delay(10);
                sdCard.writeStringLine("daten.csv", "A,B,C,D");
                delay(10);
                sdCard.writeStringLine("daten.csv", "A,B,C,D");
                delay(10);
                sdCard.writeCustomLine("daten.csv", writeCsvLine);
                delay(10);
                sdCard.writeCustomLine("daten.csv", writeCsvLine);
                delay(10);
                sdCard.writeCustomLine("daten.csv", writeCsvLine);
            } else {
                Serial.println("DEBUG: SD init FEHLER");
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
