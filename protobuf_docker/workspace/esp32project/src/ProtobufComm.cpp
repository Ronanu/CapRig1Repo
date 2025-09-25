#include "ProtobufComm.hpp"
#include <pb_encode.h>
#include <pb_decode.h>

ProtobufComm::ProtobufComm(Stream& stream)
  : serial(stream) {}

bool ProtobufComm::receive(ToEsp32& out) {
  if (serial.available() < 2) return false;
  uint16_t len = ((uint16_t)serial.read() << 8) | serial.read();
  if (len > 128) return false;

  uint8_t buffer[128];
  size_t i = 0;
  unsigned long start = micros();
  while (i < len && (micros() - start) < 50000) {
    if (serial.available()) {
      buffer[i++] = serial.read();
    }
  }

  if (i < len) return false;

  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  return pb_decode(&stream, ToEsp32_fields, &out);
}

void ProtobufComm::send(const FromEsp32& msg) {
  uint8_t buffer[128];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, FromEsp32_fields, &msg)) return;

  serial.write((uint8_t)(stream.bytes_written >> 8));
  serial.write((uint8_t)(stream.bytes_written & 0xFF));
  serial.write(buffer, stream.bytes_written);
}

void ProtobufComm::sendDebug(const char* text) {
  FromEsp32 msg = FromEsp32_init_zero;
  msg.timestamp = micros();
  msg.which_response = FromEsp32_debug_tag;
  strncpy(msg.response.debug.text, text, sizeof(msg.response.debug.text) - 1);
  msg.response.debug.text[sizeof(msg.response.debug.text) - 1] = '\0';
  send(msg);
}

uint32_t ProtobufComm::calculateChecksum(uint32_t sensor_id, float value) {
  // Simple XOR-based checksum
  union { float f; uint32_t u; } conv = { value };
  return sensor_id ^ conv.u;
}
