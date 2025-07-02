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

    // Wir wollen Kanäle 5, 6 und 7 lesen
    std::vector<uint8_t> channels = {5, 6, 7};
    reader.begin(channels, 1.0f); // Gesamt-Sampling-Rate
}

void loop() {
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 1000) { // langsamer für bessere Lesbarkeit
        lastPrint = millis();

        for (uint8_t ch : {5, 6, 7}) {
            Serial.print("Ch");
            Serial.print(ch);
            Serial.print(": ");

            const auto& buf = reader.getChannelBuffer(ch);

            // letze 10 Werte, rückwärts vom aktuellen head
            for (int i = 4; i > 0; --i) {
                size_t index = (buf.head + buf.buffer.size() - i) % buf.buffer.size();
                const auto& m = buf.buffer[index];

                Serial.print("[");
                Serial.print(m.value);
                Serial.print("@");
                Serial.print(m.timestamp);
                Serial.print("]");
                Serial.print(" ");
            }
            Serial.println();
        }
        Serial.println();
    }
}
