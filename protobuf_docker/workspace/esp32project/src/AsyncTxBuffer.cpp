#include "AsyncTxBuffer.hpp"

bool AsyncTxBuffer::begin(Stream& stream, uint16_t queueLen, uint8_t taskCore, UBaseType_t taskPrio) {
  stream_ = &stream;
  if (!queue_) queue_ = xQueueCreate(queueLen, sizeof(Frame));
  if (!queue_) return false;
  if (!task_) {
    xTaskCreatePinnedToCore(
      AsyncTxBuffer::taskTrampoline,
      "AsyncTx",
      4096,
      this,
      taskPrio,
      &task_,
      taskCore
    );
  }
  return task_ != nullptr;
}

bool AsyncTxBuffer::enqueue(const uint8_t* data, uint16_t len) {
  if (!queue_ || !stream_) return false;
  if (len == 0 || len > 128) return false;
  Frame f;
  f.len = len;
  memcpy(f.data, data, len);
  if (xQueueSend(queue_, &f, 0) == pdPASS) return true;
  Frame drop;
  xQueueReceive(queue_, &drop, 0);
  return xQueueSend(queue_, &f, 0) == pdPASS;
}

void AsyncTxBuffer::taskTrampoline(void* pv) {
  static_cast<AsyncTxBuffer*>(pv)->taskLoop();
}

void AsyncTxBuffer::taskLoop() {
  Frame f;
  for (;;) {
    if (xQueueReceive(queue_, &f, portMAX_DELAY) == pdTRUE) {
      writeFrame(f);
    }
  }
}

void AsyncTxBuffer::writeFrame(const Frame& f) {
  if (!stream_) return;
  stream_->write((uint8_t)(f.len >> 8));
  stream_->write((uint8_t)(f.len & 0xFF));
  size_t off = 0;
  while (off < f.len) {
    int n = stream_->write(f.data + off, f.len - off);
    if (n > 0) off += (size_t)n;
    else vTaskDelay(1);
  }
}
