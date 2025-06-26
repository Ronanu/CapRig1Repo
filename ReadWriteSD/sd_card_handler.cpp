#include "sd_card_handler.h"

SDCardHandler::SDCardHandler(int chipSelectPin, int cardDetectPin)
    : _chipSelectPin(chipSelectPin), _cardDetectPin(cardDetectPin), _initialized(false) {}

void SDCardHandler::begin() {
    pinMode(_cardDetectPin, INPUT);
}

bool SDCardHandler::isCardInserted() {
    return digitalRead(_cardDetectPin) == HIGH;
}

bool SDCardHandler::init() {
    _initialized = SD.begin(_chipSelectPin);
    if (!_initialized) {
        Serial.println("SD-Initialisierung fehlgeschlagen!");
    }
    return _initialized;
}

bool SDCardHandler::writeStringLine(const char* filename, const char* line) {
    if (!_initialized) return false;
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        file.println(line);
        file.close();
        return true;
    }
    return false;
}

bool SDCardHandler::writeCustomLine(const char* filename, SDWriteCallback callback) {
    if (!_initialized) return false;
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
        callback(file);
        file.close();
        return true;
    }
    return false;
}