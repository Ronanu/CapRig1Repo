#include "sd_card_handler.h"

// SD-Karten Handler mit CS Pin 16 und Card Detect Pin 17
SDCardHandler sdCard(16, 17);

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== Simple SD Card Test ===");
    Serial.println("Initialisiere SD-Karten Handler...");
    
    // Handler initialisieren
    sdCard.begin();
    
    // Prüfe ob Karte eingesteckt ist
    if (sdCard.isCardInserted()) {
        Serial.println("SD-Karte erkannt!");
        
        // SD-Karte initialisieren
        if (sdCard.init()) {
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
            
            // Teste Custom-Write mit Lambda-Funktion
            bool success = sdCard.writeCustomLine("sensordaten.csv", [](File& file) {
                unsigned long timestamp = millis();
                float temp = 25.4;
                float humidity = 60.2;
                float voltage = 3.3;
                
                file.print(timestamp);
                file.print(",");
                file.print(temp, 1);
                file.print(",");
                file.print(humidity, 1);
                file.print(",");
                file.println(voltage, 2);
            });
            
            if (success) {
                Serial.println("Sensordaten erfolgreich geschrieben!");
            } else {
                Serial.println("Fehler beim Schreiben der Sensordaten!");
            }
            
        } else {
            Serial.println("Fehler bei der SD-Karten Initialisierung!");
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
            float temperature = 20.0 + random(0, 100) / 10.0;  // 20.0 - 30.0°C
            float humidity = 40.0 + random(0, 400) / 10.0;     // 40.0 - 80.0%
            float voltage = 3.0 + random(0, 60) / 100.0;       // 3.0 - 3.6V
            
            // Schreibe neue Datenzeile
            bool success = sdCard.writeCustomLine("sensordaten.csv", [=](File& file) {
                file.print(millis());
                file.print(",");
                file.print(temperature, 1);
                file.print(",");
                file.print(humidity, 1);
                file.print(",");
                file.println(voltage, 2);
            });
            
            if (success) {
                Serial.print("Neue Daten geschrieben: ");
                Serial.print(temperature);
                Serial.print("°C, ");
                Serial.print(humidity);
                Serial.print("%, ");
                Serial.print(voltage);
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
