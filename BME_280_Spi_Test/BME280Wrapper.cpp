#include "BME280Wrapper.h"
#include <SPI.h>

BME280Wrapper::BME280Wrapper(uint8_t csPin)
    : cs(csPin), settings(csPin), bme(settings)
{
    // settings is initialized with csPin
}

bool BME280Wrapper::begin() {
    SPI.begin();
    return bme.begin();
}

void BME280Wrapper::read() {
    // Temporarily disable Serial to avoid pin conflict
    Serial.end();
    delay(100);
    
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    float dummyHumidity;
    bme.read(pressure, temperature, dummyHumidity, tempUnit, presUnit);
    
    delay(100);
    // Re-enable Serial
    Serial.begin(115200);
    delay(100); // Give Serial time to reinitialize
}

float BME280Wrapper::getTemperature() const {
    return temperature;
}

float BME280Wrapper::getPressure() const {
    return pressure;
}
