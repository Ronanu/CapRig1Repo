#ifndef PROTOBUF_COMM_HPP
#define PROTOBUF_COMM_HPP

#include <Arduino.h>
#include "messages_nanopb.pb.h"

class ProtobufComm {
public:
  ProtobufComm(Stream& stream);

  bool receive(ToEsp32& out);
  void send(const FromEsp32& msg);
  void sendDebug(const char* text);
  uint32_t calculateChecksum(uint32_t sensor_id, float value);

private:
  Stream& serial;
};

#endif
