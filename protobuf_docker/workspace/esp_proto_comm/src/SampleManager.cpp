#include "SampleManager.hpp"

SampleManager::SampleManager() : head(0) {
  memset(buffer, 0, sizeof(buffer));
}

void SampleManager::addSample(uint8_t id, float value) {
  uint32_t now = millis();
  buffer[head] = {id, value, now};
  head = (head + 1) % BUFFER_SIZE;
}

bool SampleManager::getLatest(uint8_t id, SpiSample& out) {
  for (int i = 0; i < BUFFER_SIZE; ++i) {
    size_t index = (head + BUFFER_SIZE - 1 - i) % BUFFER_SIZE;
    if (buffer[index].sensor_id == id) {
      out = buffer[index];
      return true;
    }
  }
  return false;
}
