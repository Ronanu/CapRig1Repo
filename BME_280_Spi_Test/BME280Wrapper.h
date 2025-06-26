#pragma once
#include <Arduino.h>
#include <BME280Spi.h>

class BME280Wrapper {
public:
    BME280Wrapper(uint8_t csPin);
    bool begin();
    void read();
    void printChipModel(Stream* client);

    float getTemperature() const;
    float getHumidity() const;
    float getPressure() const;

private:
    uint8_t cs;
    BME280Spi::Settings settings;
    BME280Spi bme;

    float temperature = NAN;
    float humidity = NAN;
    float pressure = NAN;
};
