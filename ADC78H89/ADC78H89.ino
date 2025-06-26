#include "ADC78H89.h"

ADC78H89 adc(0);  // CS-Pin auf D10

void setup() {
  Serial.begin(9600);
  adc.begin();
}

void loop() {
  for (uint8_t ch = 0; ch <= 7; ++ch) {
    uint16_t value = adc.readChannel(ch);
    Serial.print("AIN");
    Serial.print(ch);
    Serial.print(": ");
    Serial.print(value);
    Serial.print("\t");
    Serial.print(adc.buildControlByte(ch), BIN);
    Serial.print("\t");
    Serial.println(value * 5 / 4095.0);
  }

  Serial.println("------");
  delay(100);  // 1 Sekunde Pause zwischen den Zyklen
}
