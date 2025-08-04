#include "CommandHandler.hpp"
#include <pb_encode.h>

extern uint32_t calculateHash(const uint8_t* data, size_t length);

CommandHandler::CommandHandler(SampleManager& sm, Stream& s)
    : sampleManager(sm), serial(s) {}

void CommandHandler::dispatch(const ToEsp32& msg) {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = millis();

  // Fülle ACK-Nachricht
  strncpy(response.payload.ack.message, "ACK", sizeof(response.payload.ack.message) - 1);
  response.payload.ack.message[sizeof(response.payload.ack.message) - 1] = '\0';
  response.which_payload = FromEsp32_ack_tag;

  // Hash vorbereiten
  response.hash = 0;
  uint8_t tempBuf[128];
  pb_ostream_t tempStream = pb_ostream_from_buffer(tempBuf, sizeof(tempBuf));
  if (pb_encode(&tempStream, FromEsp32_fields, &response)) {
    response.hash = calculateHash(tempBuf, tempStream.bytes_written);
  }

  // Final codieren und senden
  pb_ostream_t stream = pb_ostream_from_buffer(tempBuf, sizeof(tempBuf));
  if (!pb_encode(&stream, FromEsp32_fields, &response)) return;

  serial.write((uint8_t)(stream.bytes_written >> 8));
  serial.write((uint8_t)(stream.bytes_written & 0xFF));
  serial.write(tempBuf, stream.bytes_written);
}
