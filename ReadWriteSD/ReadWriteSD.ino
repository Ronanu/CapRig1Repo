#include "sd_card_handler.h"

// SD-Karten Handler mit CS Pin 5 und Card Detect Pin 16
SDCardHandler sdCard(5, 17); // CS auf Pin 5, Card Detect auf Pin 17

// Globale Variablen für die Callback-Funktion
float currentTemp, currentHumidity, currentVoltage;

// Callback-Funktion für das Schreiben der Sensordaten
void writeSensorData(File& file) {
    file.print(millis());
    file.print(",");
    file.print(currentTemp, 1);
    file.print(",");
    file.print(currentHumidity, 1);
    file.print(",");
    file.println(currentVoltage, 2);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== SD Card Test ===");
    
    // Hardware-Setup (Pin-Konfiguration)
    Serial.println("Init...");
    sdCard.init();
    Serial.println("Init OK");
    
    // SD-Karte initialisieren
    Serial.println("Begin...");
    if (!sdCard.begin()) {
        Serial.println("Begin FEHLER!");
        return;
    }
    Serial.println("Begin OK");
    
    // Prüfe ob Karte eingesteckt ist
    if (sdCard.isCardInserted()) {
        Serial.println("SD-Karte erkannt!");
        Serial.println("SD-Karte erfolgreich initialisiert!");
            
            // Teste einfachen String-Write
            if (sdCard.writeStringLine("test.txt", "Hallo SD-Karte!")) {
                Serial.println("Test-String erfolgreich geschrieben!");
            } else {
                Serial.println("Fehler beim Schreiben des Test-Strings!");
            }
            
            // Teste CSV-Header schreiben
            if (sdCard.writeStringLine("sensordaten.csv", "Zeit,Temperatur,Luftfeuchtigkeit,Spannung")) {
                Serial.println("CSV-Header erfolgreich geschrieben!");
            }
            
            // Teste Custom-Write mit Callback-Funktion
            currentTemp = 25.4;
            currentHumidity = 60.2;
            currentVoltage = 3.3;
            
            bool success = sdCard.writeCustomLine("sensordaten.csv", writeSensorData);
            
            if (success) {
                Serial.println("Sensordaten erfolgreich geschrieben!");
            } else {
                Serial.println("Fehler beim Schreiben der Sensordaten!");
            }
            
    } else {
        Serial.println("Keine SD-Karte erkannt!");
        Serial.println("Bitte SD-Karte einsetzen und neustarten.");
    }
    
    Serial.println("Setup abgeschlossen!");
}

void loop() {
    // Alle 10 Sekunden neue Sensordaten schreiben
    static unsigned long lastWrite = 0;
    
    if (millis() - lastWrite >= 10000) {
        
        // Nur schreiben wenn SD-Karte verfügbar ist
        if (sdCard.isCardInserted()) {
            
            // Simuliere Sensordaten
            currentTemp = 20.0 + random(0, 100) / 10.0;     // 20.0 - 30.0°C
            currentHumidity = 40.0 + random(0, 400) / 10.0; // 40.0 - 80.0%
            currentVoltage = 3.0 + random(0, 60) / 100.0;   // 3.0 - 3.6V
            
            // Schreibe neue Datenzeile mit Callback-Funktion
            bool success = sdCard.writeCustomLine("sensordaten.csv", writeSensorData);
            
            if (success) {
                Serial.print("Neue Daten geschrieben: ");
                Serial.print(currentTemp);
                Serial.print("°C, ");
                Serial.print(currentHumidity);
                Serial.print("%, ");
                Serial.print(currentVoltage);
                Serial.println("V");
            } else {
                Serial.println("Fehler beim Schreiben der Daten!");
            }
            
        } else {
            Serial.println("SD-Karte nicht verfügbar!");
        }
        
        lastWrite = millis();
    }
    
    // Kurze Pause
    delay(100);
}
