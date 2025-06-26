#pragma once
#include <Arduino.h>
#include <SD.h>

typedef void (*SDWriteCallback)(File&);

class SDCardHandler {
public:
    SDCardHandler(int chipSelectPin, int cardDetectPin);
    void begin();
    bool isCardInserted();
    bool init();
    bool writeStringLine(const char* filename, const char* line);
    bool writeCustomLine(const char* filename, SDWriteCallback callback);
    // ... weitere Methoden wie readLines, deleteFile ...
private:
    int _chipSelectPin;
    int _cardDetectPin;
    bool _initialized;
};