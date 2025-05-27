#include "ADC78H89.h"
#define MUX3_EN 10
#define MUX3_AD0 11
#define MUX3_AD1 12

ADC78H89 adc(0);  // CS-Pin auf D10

void setup() {
  Serial.begin(9600);
  pinMode(MUX3_EN, OUTPUT);
  pinMode(MUX3_AD0, OUTPUT);
  pinMode(MUX3_AD1, OUTPUT);
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
  digitalWrite(MUX3_EN, HIGH);
  digitalWrite(MUX3_AD0, LOW);
  digitalWrite(MUX3_AD1, HIGH);
  
  
  
  Serial.println("------");
  delay(100);  // 1 Sekunde Pause zwischen den Zyklen
}
