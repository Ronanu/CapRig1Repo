#include "ProtobufComm.hpp"
#include <pb_encode.h>
#include <pb_decode.h>

ProtobufComm::ProtobufComm(Stream& stream, SampleManager& sm) : serial(stream), sampleManager(sm) {}

uint32_t ProtobufComm::calculateHash(const uint8_t* data, size_t length) {
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < length; i++) {
    hash ^= data[i];
    hash *= 16777619u;
  }
  return hash;
}

void ProtobufComm::sendMessage(const FromEsp32& msg) {
  uint8_t buffer[128];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  if (!pb_encode(&stream, FromEsp32_fields, &msg)) {
    serial.println("Encoding failed!");
    return;
  }

  serial.write((uint8_t)(stream.bytes_written >> 8));
  serial.write((uint8_t)(stream.bytes_written & 0xFF));
  serial.write(buffer, stream.bytes_written);
}

void ProtobufComm::sendStatus() {
  FromEsp32 msg = FromEsp32_init_zero;
  msg.timestamp = millis();

  SpiSample s;
  if (sampleManager.getLatest(0, s)) {
    msg.value1 = s.value;
  }
  if (sampleManager.getLatest(1, s)) {
    msg.value2 = s.value;
  }

  msg.hash = 0;

  uint8_t tempBuf[128];
  pb_ostream_t tempStream = pb_ostream_from_buffer(tempBuf, sizeof(tempBuf));
  if (!pb_encode(&tempStream, FromEsp32_fields, &msg)) return;
  msg.hash = calculateHash(tempBuf, tempStream.bytes_written);

  sendMessage(msg);
}

bool ProtobufComm::receive(ToEsp32& out) {
  if (serial.available() < 2) return false;
  uint16_t len = ((uint16_t)serial.read() << 8) | serial.read();
  if (len > 128) return false;

  uint8_t buffer[128];
  size_t i = 0;
  while (i < len) {
    if (serial.available()) buffer[i++] = serial.read();
  }

  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  return pb_decode(&stream, ToEsp32_fields, &out);
}

void ProtobufComm::handle(const ToEsp32& msg) {
  // Placeholder: Aktuell nur get_status
  if (strcmp(msg.command, "get_status") == 0) {
    sendStatus();
  }
}
