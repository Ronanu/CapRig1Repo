#pragma once
#include <Arduino.h>
#include "SampleManager.hpp"
#include "messages.pb.h"

class ProtobufComm {
public:
  ProtobufComm(Stream& stream, SampleManager& sm);
  void sendStatus();
  void receiveAndHandle();

private:
  Stream& serial;
  SampleManager& sampleManager;
  uint32_t calculateHash(const uint8_t* data, size_t length);
  void sendMessage(const FromEsp32& msg);
};
