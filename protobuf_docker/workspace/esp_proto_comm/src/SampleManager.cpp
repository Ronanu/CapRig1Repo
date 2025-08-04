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

bool SampleManager::getAverage(uint8_t id, size_t count, float& average) {
  float sum = 0.0f;
  size_t found = 0;
  for (int i = 0; i < BUFFER_SIZE && found < count; ++i) {
    size_t index = (head + BUFFER_SIZE - 1 - i) % BUFFER_SIZE;
    if (buffer[index].sensor_id == id) {
      sum += buffer[index].value;
      ++found;
    }
  }
  if (found == 0) return false;
  average = sum / found;
  return true;
}
