#include "AsyncPacketBuffer.hpp"
#include <string.h>

namespace AsyncPacketBuffer {

struct TxPacket {
  uint16_t len;
  uint8_t  data[128];
};

struct Handle {
  Stream*       stream = nullptr;
  QueueHandle_t q      = nullptr;
  TaskHandle_t  task   = nullptr;
};

static Handle* g_handle = nullptr;

static void txTask(void* pv) {
  Handle* h = static_cast<Handle*>(pv);
  TxPacket pkt;
  for (;;) {
    if (xQueueReceive(h->q, &pkt, portMAX_DELAY) == pdTRUE) {
      // Write length (big endian)
      h->stream->write((uint8_t)(pkt.len >> 8));
      h->stream->write((uint8_t)(pkt.len & 0xFF));
      // Write payload handling partial writes
      size_t off = 0;
      while (off < pkt.len) {
        int n = h->stream->write(pkt.data + off, pkt.len - off);
        if (n > 0) off += (size_t)n;
        else vTaskDelay(1);
      }
    }
  }
}

Handle* begin(Stream& stream, uint8_t taskCore, UBaseType_t taskPrio, uint16_t queueLen) {
  if (g_handle) return g_handle;
  Handle* h = new Handle();
  h->stream = &stream;
  h->q = xQueueCreate(queueLen, sizeof(TxPacket));
  if (!h->q) { delete h; return nullptr; }

  BaseType_t ok = xTaskCreatePinnedToCore(
    txTask,
    "AsyncPktTx",
    4096,
    h,
    taskPrio,
    &h->task,
    taskCore
  );
  if (ok != pdPASS) {
    vQueueDelete(h->q);
    delete h;
    return nullptr;
  }
  g_handle = h;
  return g_handle;
}

bool send(const uint8_t* data, uint16_t len) {
  if (!g_handle || !g_handle->q) return false;
  if (len == 0 || len > 128) return false;

  TxPacket pkt;
  pkt.len = len;
  memcpy(pkt.data, data, len);

  if (xQueueSend(g_handle->q, &pkt, 0) == pdPASS) {
    return true;
  }
  // Drop-oldest on overflow
  TxPacket drop;
  xQueueReceive(g_handle->q, &drop, 0);
  return xQueueSend(g_handle->q, &pkt, 0) == pdPASS;
}

bool isActive() {
  return g_handle && g_handle->q && g_handle->task;
}

void end() {
  // Not implemented for simplicity; typical Arduino apps don't stop tasks.
}

} // namespace AsyncPacketBuffer
