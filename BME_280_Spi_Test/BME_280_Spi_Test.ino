#include "BME280Wrapper.h"

#define SERIAL_BAUD 115200
#define DEVICE_PIN 1

BME280Wrapper bme(DEVICE_PIN);

void setup() {
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {}

    Serial.println("Setup start");

    if (bme.begin()) {
        Serial.println("Sensor initialized.");
    } else {
        Serial.println("Sensor init failed!");
    }
}

void loop() {
    Serial.println("Loop running");

    bme.read();  // 1x pro Loop aktualisieren

    float temp = bme.getTemperature();
    float hum = bme.getHumidity();
    float pres = bme.getPressure();

    Serial.print("Temperature: ");
    Serial.print(temp);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.println(" %");

    Serial.print("Pressure: ");
    Serial.print(pres);
    Serial.println(" Pa");

    // Debug: Check for NaN values
    if (isnan(temp) || isnan(hum) || isnan(pres)) {
        Serial.println("WARNING: Some readings are invalid (NaN)");
    }

    Serial.println("-----------------------------");
    delay(1000);
}
