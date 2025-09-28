#ifndef ASYNC_TX_BUFFER_HPP
#define ASYNC_TX_BUFFER_HPP

#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

class AsyncTxBuffer {
public:
  AsyncTxBuffer() = default;
  bool begin(Stream& stream, uint16_t queueLen = 16, uint8_t taskCore = 1, UBaseType_t taskPrio = 1);
  bool enqueue(const uint8_t* data, uint16_t len);

private:
  struct Frame {
    uint16_t len = 0;
    uint8_t  data[128];
  };

  static void taskTrampoline(void* pv);
  void taskLoop();
  void writeFrame(const Frame& f);

  Stream*       stream_   = nullptr;
  QueueHandle_t queue_    = nullptr;
  TaskHandle_t  task_     = nullptr;
};

#endif // ASYNC_TX_BUFFER_HPP
