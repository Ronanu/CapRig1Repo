#include "sd_card_handler.h"

SDCardHandler::SDCardHandler(int chipSelectPin, int cardDetectPin)
    : _chipSelectPin(chipSelectPin), _cardDetectPin(cardDetectPin), _initialized(false) {}


bool SDCardHandler::isCardInserted() {
    return digitalRead(_cardDetectPin) == HIGH;
}

void SDCardHandler::init() {
    // Pin-Konfiguration (Hardware-Setup)
    pinMode(_cardDetectPin, INPUT);
}

bool SDCardHandler::begin() {
    // SD-Karte initialisieren
    Serial.print("CS Pin: ");
    Serial.println(_chipSelectPin);
    Serial.print("Card Detect: ");
    Serial.println(digitalRead(_cardDetectPin));
    
    // SPI explizit initialisieren
    SPI.begin();
    
    _initialized = SD.begin(_chipSelectPin);
    Serial.print("SD.begin result: ");
    Serial.println(_initialized);
    
    delay(100);
    return _initialized;
}

bool SDCardHandler::writeStringLine(const char* filename, const char* line) {
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.println(line);
        file.close();
        return true;
    }
    return false;
}

bool SDCardHandler::writeCustomLine(const char* filename, SDWriteCallback callback) {
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        callback(file);
        file.close();
        return true;
    }
    return false;
}