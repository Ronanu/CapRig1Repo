#include <SPI.h>

// Pin-Definitionen für MicroSD Breakout Board
const int chipSelect = 10;    // CS Pin für SD-Karte
bool testExecuted = false;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== GRUNDLEGENDER SPI TEST (OHNE SD LIBRARY) ===");
    
    // CS Pin konfigurieren
    pinMode(chipSelect, OUTPUT);
    digitalWrite(chipSelect, HIGH);
    
    // SPI initialisieren
    SPI.begin();
    
    Serial.println("Verkabelung prüfen:");
    Serial.println("VCC -> 3.3V oder 5V");
    Serial.println("GND -> GND");  
    Serial.println("MISO -> Pin 12");
    Serial.println("MOSI -> Pin 11");
    Serial.println("SCK -> Pin 13");
    Serial.println("CS -> Pin A5");
    Serial.println("\nTest wird im loop() ausgeführt...");
}

void loop() {
    if (!testExecuted) {
        testExecuted = true;
        
        Serial.println("\nHARDWARE DIAGNOSE...");
        
        // MISO Pin Test ohne SPI
        Serial.print("MISO Pin Test (ohne SPI): ");
        Serial.println(digitalRead(12) ? "HIGH" : "LOW");
        
        // MISO mit Pull-up Test
        pinMode(12, INPUT_PULLUP);
        delay(10);
        Serial.print("MISO mit Pull-up: ");
        Serial.println(digitalRead(12) ? "HIGH" : "LOW");
        
        // Zurück zu normalem Input
        pinMode(12, INPUT);
        
        Serial.println("\nSPI TEST mit verschiedenen Modi...");
        
        // Test 1: SPI Mode 0, langsam
        Serial.println("Test 1: SPI Mode 0, 100kHz");
        testSPI(SPI_MODE0, 100000);
        
        // Test 2: SPI Mode 1
        Serial.println("Test 2: SPI Mode 1, 100kHz");
        testSPI(SPI_MODE1, 100000);
        
        // Test 3: Noch langsamer
        Serial.println("Test 3: SPI Mode 0, 50kHz");
        testSPI(SPI_MODE0, 50000);
        
        Serial.println("\n=== ALLE TESTS BEENDET ===");
        Serial.println("Falls überall 0x0: MISO wahrscheinlich auf GND kurzgeschlossen!");
    }
}

void testSPI(int spiMode, long frequency) {
    SPI.beginTransaction(SPISettings(frequency, MSBFIRST, spiMode));
    
    // CS aktivieren (LOW)
    digitalWrite(chipSelect, LOW);
    delay(10); // Längere Pause
    
    // Erst ein paar 0xFF senden (für SD-Karte bereit machen)
    for (int i = 0; i < 10; i++) {
        SPI.transfer(0xFF);
    }
    
    // CMD0 senden
    SPI.transfer(0x40); // Command 0
    SPI.transfer(0x00); // Argument
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x95); // CRC
    
    // Antwort lesen
    byte response = 0xFF;
    for (int i = 0; i < 10; i++) {
        response = SPI.transfer(0xFF);
        Serial.print("  Antwort "); Serial.print(i+1); Serial.print(": 0x");
        Serial.println(response, HEX);
        
        if (response == 0x01) {
            Serial.println("  SUCCESS!");
            break;
        } else if (response != 0xFF && response != 0x00) {
            Serial.print("  Unerwartete aber gültige Antwort: 0x");
            Serial.println(response, HEX);
            break;
        }
    }
    
    // CS deaktivieren (HIGH)
    digitalWrite(chipSelect, HIGH);
    SPI.endTransaction();
    delay(100);
}
