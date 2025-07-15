#pragma once
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

typedef void (*SDWriteCallback)(File&);

class SDCardHandler {
public:
    SDCardHandler(int chipSelectPin, int cardDetectPin);
    void init();
    bool begin();
    bool isCardInserted();
    bool writeStringLine(const char* filename, const char* line);
    bool writeCustomLine(const char* filename, SDWriteCallback callback);
    // ... weitere Methoden wie readLines, deleteFile ...
private:
    int _chipSelectPin;
    int _cardDetectPin;
    bool _initialized;
};