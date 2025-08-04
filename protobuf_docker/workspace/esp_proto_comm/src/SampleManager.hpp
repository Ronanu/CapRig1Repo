#pragma once
#include <Arduino.h>

struct SpiSample {
  uint8_t sensor_id;
  float value;
  uint32_t timestamp;
};

constexpr size_t BUFFER_SIZE = 1024;

class SampleManager {
public:
  SampleManager();
  void addSample(uint8_t id, float value);
  bool getLatest(uint8_t id, SpiSample& out);

private:
  SpiSample buffer[BUFFER_SIZE];
  size_t head;
};
