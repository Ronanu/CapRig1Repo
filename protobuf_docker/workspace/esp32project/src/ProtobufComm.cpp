#include "ProtobufComm.hpp"
#include <pb_encode.h>
#include <pb_decode.h>
#include "AsyncPacketBuffer.hpp"

ProtobufComm::ProtobufComm(Stream& stream)
  : serial(stream) {}

bool ProtobufComm::receive(ToEsp32& out) {
  if (serial.available() < 2) return false;
  uint16_t len = ((uint16_t)serial.read() << 8) | serial.read();
  if (len == 0 || len > 128) return false;

  uint8_t buffer[128];
  size_t i = 0;
  unsigned long start = micros();
  while (i < len) {
    if (serial.available()) {
      buffer[i++] = (uint8_t)serial.read();
    } else if (micros() - start > 20000) { // ~20ms guard
      return false;
    } else {
      vTaskDelay(1);
    }
  }

  pb_istream_t istream = pb_istream_from_buffer(buffer, len);
  return pb_decode(&istream, ToEsp32_fields, &out);
}

void ProtobufComm::send(const FromEsp32& msg) {
  uint8_t buffer[128];
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&stream, FromEsp32_fields, &msg)) return;

  const uint16_t n = (uint16_t)stream.bytes_written;
  if (AsyncPacketBuffer::isActive()) {
    if (AsyncPacketBuffer::send(buffer, n)) {
      return;
    }
    // Queue full -> guarantee error message with same seq (sync path)
    FromEsp32 err = FromEsp32_init_zero;
    err.seq = msg.seq;
    err.timestamp = micros();
    err.which_response = FromEsp32_error_tag;
    strncpy(err.response.error.error, "tx_queue_full", sizeof(err.response.error.error) - 1);

    uint8_t ebuf[128];
    pb_ostream_t es = pb_ostream_from_buffer(ebuf, sizeof(ebuf));
    if (pb_encode(&es, FromEsp32_fields, &err)) {
      const uint16_t en = (uint16_t)es.bytes_written;
      serial.write((uint8_t)(en >> 8));
      serial.write((uint8_t)(en & 0xFF));
      serial.write(ebuf, en);
    }
    return;
  }

  // Fallback: synchronous
  serial.write((uint8_t)(n >> 8));
  serial.write((uint8_t)(n & 0xFF));
  serial.write(buffer, n);
}

void ProtobufComm::sendDebug(const char* text) {
  FromEsp32 msg = FromEsp32_init_zero;
  msg.seq = 0;
  msg.timestamp = micros();
  msg.which_response = FromEsp32_debug_tag;
  strncpy(msg.response.debug.text, text, sizeof(msg.response.debug.text) - 1);
  msg.response.debug.text[sizeof(msg.response.debug.text) - 1] = '\0';
  send(msg);
}

uint32_t ProtobufComm::calculateChecksum(uint32_t sensor_id, float value) {
  union { float f; uint32_t u; } conv = { value };
  return sensor_id ^ conv.u;
}
