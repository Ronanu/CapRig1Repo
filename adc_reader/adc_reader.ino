#include "ADC78H89.h"
#include "TimerCallback.h"
#include "ADCReader.h"

// Pinout je nach Board
#define CS_PIN A1

ADC78H89 adc(CS_PIN);
TimerCallback timer;
ADCReader reader(&adc, &timer);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    adc.begin();

    // Wir wollen Kanäle 1, 2 und 5 lesen
    std::vector<uint8_t> channels = {1, 6, 7};
    reader.begin(channels, 100.0f); // max Gesamtsampling-Rate
}

void loop() {
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 100) {
        lastPrint = millis();

        for (uint8_t ch : {1, 6, 7}) {
            const auto& buf = reader.getChannelBuffer(ch);
            size_t lastIndex = (buf.head + buf.buffer.size() - 1) % buf.buffer.size();
            const auto& m = buf.buffer[lastIndex];

            Serial.print("Ch");
            Serial.print(ch);
            Serial.print(": ");
            Serial.print(m.value);
            Serial.print(" @ ");
            Serial.print(m.timestamp);
            Serial.print(" us | ");
        }
        Serial.println();
    }
}
