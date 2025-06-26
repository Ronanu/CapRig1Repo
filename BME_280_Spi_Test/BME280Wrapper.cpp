#include "BME280Wrapper.h"
#include <SPI.h>

BME280Wrapper::BME280Wrapper(uint8_t csPin)
    : cs(csPin), settings(csPin), bme(settings)
{
    // settings is initialized with csPin
}

bool BME280Wrapper::begin() {
    SPI.begin();
    int tries = 0;
    while (!bme.begin() && tries < 5) {
        Serial.println("Could not find BME280 sensor! Retrying...");
        delay(1000);
        tries++;
    }
    if (tries == 5) {
        Serial.println("BME280 initialization failed!");
        return false;
    }
    printChipModel(&Serial);
    delay(100);
    Serial.println("BME280 initialized successfully");
    return true;
}

void BME280Wrapper::printChipModel(Stream* client) {
    switch (bme.chipModel()) {
        case BME280::ChipModel_BME280:
            client->println("Found BME280 sensor! Success.");
            break;
        case BME280::ChipModel_BMP280:
            client->println("Found BMP280 sensor! No Humidity available.");
            break;
        default:
            client->println("Found UNKNOWN sensor! Error!");
    }
}

void BME280Wrapper::read() {
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    bme.read(pressure, temperature, humidity, tempUnit, presUnit);
    // Validate readings - BME280 has specific ranges
    if (temperature < -40.0 || temperature > 85.0) {
        Serial.println("Temperature reading out of range!");
        temperature = NAN;
    }
    if (humidity < 0.0 || humidity > 100.0) {
        Serial.println("Humidity reading out of range!");
        humidity = NAN;
    }
    if (pressure < 30000.0 || pressure > 110000.0) {
        Serial.println("Pressure reading out of range!");
        pressure = NAN;
    }
    if (temperature == 0.0 && humidity == 0.0 && pressure == 0.0) {
        Serial.println("All readings are 0.0 - possible communication issue!");
    }
}

float BME280Wrapper::getTemperature() const {
    return temperature;
}

float BME280Wrapper::getHumidity() const {
    return humidity;
}

float BME280Wrapper::getPressure() const {
    return pressure;
}
