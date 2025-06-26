#pragma once
#include <Arduino.h>
#include <BME280Spi.h>

class BME280Wrapper {
public:
    BME280Wrapper(uint8_t csPin);
    bool begin();
    void read();

    float getTemperature() const;
    float getPressure() const;

private:
    uint8_t cs;
    BME280Spi::Settings settings;
    BME280Spi bme;

    float temperature = NAN;
    float pressure = NAN;
};
