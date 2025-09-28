#include "ProtoTxBuffer.hpp"

static QueueHandle_t s_txQueue = nullptr;
static TaskHandle_t  s_txTask  = nullptr;
static Stream*       s_stream  = nullptr;

struct TxPacket {
  uint16_t len;
  uint8_t  data[128];
};

static void writePacket(Stream& s, const TxPacket& pkt) {
  s.write((uint8_t)(pkt.len >> 8));
  s.write((uint8_t)(pkt.len & 0xFF));
  size_t off = 0;
  while (off < pkt.len) {
    int n = s.write(pkt.data + off, pkt.len - off);
    if (n > 0) {
      off += (size_t)n;
    } else {
      vTaskDelay(1);
    }
  }
}

static void txTask(void* pv) {
  (void)pv;
  TxPacket pkt;
  for (;;) {
    if (xQueueReceive(s_txQueue, &pkt, portMAX_DELAY) == pdTRUE) {
      writePacket(*s_stream, pkt);
    }
  }
}

bool proto_txbuffer_begin(Stream& serial, uint8_t taskCore, UBaseType_t taskPrio, uint16_t queueLen) {
  if (!s_txQueue) {
    s_txQueue = xQueueCreate(queueLen, sizeof(TxPacket));
  }
  s_stream = &serial;
  if (s_txQueue && !s_txTask) {
    xTaskCreatePinnedToCore(txTask, "ProtoTx", 4096, nullptr, taskPrio, &s_txTask, taskCore);
  }
  return s_txQueue && s_txTask;
}

bool proto_txbuffer_enqueue(const uint8_t* data, uint16_t len) {
  if (!s_txQueue || !s_stream || len == 0 || len > 128) return false;
  TxPacket pkt;
  pkt.len = len;
  memcpy(pkt.data, data, len);
  if (xQueueSend(s_txQueue, &pkt, 0) == pdPASS) return true;
  TxPacket drop;
  (void)xQueueReceive(s_txQueue, &drop, 0);
  return xQueueSend(s_txQueue, &pkt, 0) == pdPASS;
}

bool proto_txbuffer_active() {
  return s_txQueue != nullptr && s_txTask != nullptr && s_stream != nullptr;
}
