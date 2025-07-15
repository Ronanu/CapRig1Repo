#include "sd_card_handler.h"

SDCardHandler::SDCardHandler(int chipSelectPin, int cardDetectPin)
    : _chipSelectPin(chipSelectPin), _cardDetectPin(cardDetectPin), _initialized(false) {}


bool SDCardHandler::isCardInserted() {
    return true; //digitalRead(_cardDetectPin) == HIGH;
}

bool SDCardHandler::init() {
    // Pin-Konfiguration (Hardware-Setup)
    pinMode(_cardDetectPin, INPUT);
    pinMode(_chipSelectPin, OUTPUT);
    digitalWrite(_chipSelectPin, HIGH); // CS initial deaktiviert
    
    // SD-Karte initialisieren
    _initialized = SD.begin(_chipSelectPin);
    delay(100);
    return _initialized;
}

bool SDCardHandler::writeStringLine(const char* filename, const char* line) {
    digitalWrite(_chipSelectPin, LOW);  // CS aktivieren
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.println("hallo");
        file.close();
        digitalWrite(_chipSelectPin, HIGH); // CS deaktivieren
        return true;
    }
    digitalWrite(_chipSelectPin, HIGH); // CS deaktivieren bei Fehler
    return false;
}

bool SDCardHandler::writeCustomLine(const char* filename, SDWriteCallback callback) {
    digitalWrite(_chipSelectPin, LOW);  // CS aktivieren
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        callback(file);
        file.close();
        digitalWrite(_chipSelectPin, HIGH); // CS deaktivieren
        return true;
    }
    digitalWrite(_chipSelectPin, HIGH); // CS deaktivieren bei Fehler
    return false;
}