#include "ProtobufComm.hpp"
#include <pb_encode.h>
#include <pb_decode.h>

ProtobufComm::ProtobufComm(Stream& stream, SampleManager& sm)
  : serial(stream), sampleManager(sm) {}

uint32_t ProtobufComm::calculateHash(const uint8_t* data, size_t length) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < length; i++) {
    hash ^= data[i];
    hash *= 16777619u;
  }
  return hash;
}

bool ProtobufComm::receive(ToEsp32& out) {
  if (serial.available() < 2) return false;
  uint16_t len = ((uint16_t)serial.read() << 8) | serial.read();
  if (len > 128) return false;

  uint8_t buffer[128];
  size_t i = 0;
  unsigned long start = millis();
  while (i < len && (millis() - start) < 50) {
    if (serial.available()) buffer[i++] = serial.read();
  }

  if (i < len) return false;

  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  return pb_decode(&stream, ToEsp32_fields, &out);
}

void ProtobufComm::send(const FromEsp32& msgIn) {
  FromEsp32 msg = msgIn;
  msg.hash = 0;

  uint8_t buffer[128];
  pb_ostream_t tempStream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&tempStream, FromEsp32_fields, &msg)) return;

  msg.hash = calculateHash(buffer, tempStream.bytes_written);
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, FromEsp32_fields, &msg)) return;

  serial.write((uint8_t)(stream.bytes_written >> 8));
  serial.write((uint8_t)(stream.bytes_written & 0xFF));
  serial.write(buffer, stream.bytes_written);
}
