#pragma once
#include "messages.pb.h"
#include "SampleManager.hpp"
#include "ProtobufComm.hpp"

class CommandHandler {
public:
  CommandHandler(SampleManager& sm, ProtobufComm& pc);
  void dispatch(const ToEsp32& msg);

private:
  SampleManager& sampleManager;
  ProtobufComm& protoComm;
};
