#pragma once
#include <Arduino.h>
#include "SampleManager.hpp"
#include "messages.pb.h"

#include "CommandHandler.hpp"

class ProtobufComm {
public:
  ProtobufComm(Stream& stream, SampleManager& sm);
  void sendStatus();
  bool receive(ToEsp32& out);
  void handle(const ToEsp32& msg);

public:
  void setDispatcher(CommandHandler* handler);

private:
  Stream& serial;
  SampleManager& sampleManager;
  CommandHandler* dispatcher = nullptr;
  uint32_t calculateHash(const uint8_t* data, size_t length);
  void sendMessage(const FromEsp32& msg);
};
