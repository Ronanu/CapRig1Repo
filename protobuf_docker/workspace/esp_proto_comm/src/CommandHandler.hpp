#pragma once
#include "messages.pb.h"
#include "SampleManager.hpp"
#include <Arduino.h>

class CommandHandler {
public:
  CommandHandler(SampleManager& sm, Stream& serial);
  void dispatch(const ToEsp32& msg);

private:
  SampleManager& sampleManager;
  Stream& serial;
};
