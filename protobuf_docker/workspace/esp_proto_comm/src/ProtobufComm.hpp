#pragma once
#include <Arduino.h>
#include "SampleManager.hpp"
#include "messages.pb.h"

#include "CommandHandler.hpp"

class ProtobufComm {
public:
  ProtobufComm(Stream& stream, SampleManager& sm);
  bool receive(ToEsp32& out);
  void send(const FromEsp32& msg);

private:
  Stream& serial;
  SampleManager& sampleManager;
  uint32_t calculateHash(const uint8_t* data, size_t length);
};
