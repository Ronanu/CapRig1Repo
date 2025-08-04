#include "CommandHandler.hpp"

CommandHandler::CommandHandler(SampleManager& sm, ProtobufComm& pc)
    : sampleManager(sm), protoComm(pc) {}

void CommandHandler::dispatch(const ToEsp32& msg) {
  // Beispiel: Nur "get_status" behandeln
  if (msg.has_request_status) {
    protoComm.sendStatus();
  }
}
