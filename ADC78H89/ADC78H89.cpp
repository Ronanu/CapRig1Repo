#include "ADC78H89.h"

ADC78H89::ADC78H89(uint8_t csPin) : _csPin(csPin) {}

void ADC78H89::begin() {
  pinMode(_csPin, OUTPUT);
  digitalWrite(_csPin, HIGH);
  SPI.begin();
}

uint8_t ADC78H89::buildControlByte(uint8_t channel) {
  return (channel | 0x00) << 3;
}

uint16_t ADC78H89::readChannel(uint8_t channel) {
  uint8_t controlByte = buildControlByte(channel);
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  delayMicroseconds(1);  
  digitalWrite(_csPin, LOW);
  delayMicroseconds(2);
  // Sende 8 Kontrollbits + empfange gleichzeitig die ersten 8 Bits (4x 0 + 4 Datenbits)
  uint8_t highByte = SPI.transfer(controlByte);
  // Empfang zweite Hälfte (restliche 8 Datenbits)
  uint8_t lowByte = SPI.transfer(0x00);
  digitalWrite(_csPin, HIGH);
  SPI.endTransaction();
  delayMicroseconds(1);

  // Ergebnis besteht aus 12 Bit ab Bit 4 von highByte
  uint16_t result = ((highByte & 0x0F) << 8) | lowByte;
  return result;
}
