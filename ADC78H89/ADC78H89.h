#pragma once
#include <SPI.h>

class ADC78H89 {
public:
  ADC78H89(uint8_t csPin);
  void begin();
  uint16_t readChannel(uint8_t channel);  // 0–6 (AIN1–AIN7)
  uint8_t buildControlByte(uint8_t channel);

private:
  uint8_t _csPin;
};
