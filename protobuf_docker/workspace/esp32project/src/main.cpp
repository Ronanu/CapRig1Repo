#include <Arduino.h>
#include "ProtobufComm.hpp"
#include "Settings.hpp"
#include "messages_nanopb.pb.h"
#include "AsyncPacketBuffer.hpp"

constexpr TickType_t RX_TASK_DELAY = pdMS_TO_TICKS(1);

ProtobufComm protoComm(Serial);


static void sendAck(const char* msg, uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_ack_tag;
  strncpy(res.response.ack.message, msg, sizeof(res.response.ack.message)-1);
  protoComm.send(res);
}

static void sendError(const char* msg, uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_error_tag;
  strncpy(res.response.error.error, msg, sizeof(res.response.error.error)-1);
  protoComm.send(res);
}

static void sendSettings(uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_settings_tag;
  SettingsManager::instance().toProto(res.response.settings.settings);
  protoComm.send(res);
}
static void sendSample(uint32_t sensor_id, float value, uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_sample_tag;
  res.response.sample.sensor_id = sensor_id;
  res.response.sample.value = value;
  // Simple checksum (same as helper)
  union { float f; uint32_t u; } conv = { value };
  res.response.sample.checksum = sensor_id ^ conv.u;
  protoComm.send(res);
}

void rxTask(void* pv) {
  for (;;) {
    ToEsp32 cmd = ToEsp32_init_zero;
    if (protoComm.receive(cmd)) {
      uint32_t seq = cmd.seq;
      switch (cmd.which_command) {
        case ToEsp32_ping_tag: {
          sendAck("pong", seq);
          break;
        }
        case ToEsp32_get_settings_tag: {
          sendSettings(seq);
          break;
        }
        case ToEsp32_set_settings_tag: {
          SettingsManager::instance().fromProto(cmd.command.set_settings.settings);
          SettingsManager::instance().saveDebounced();
          sendAck("settings updated", seq);
          sendSettings(seq);
          break;
        }
        case ToEsp32_set_mux_tag: {
          uint32_t ch = cmd.command.set_mux.channel;
          (void)ch; // TODO: implement MUX switching
          sendAck("mux set", seq);
          break;
        }
        default: {
          sendError("unknown command", seq);
          break;
        }
      }
    } else {
      vTaskDelay(RX_TASK_DELAY);
    }
    SettingsManager::instance().tick();
  }
}
 void setup(){
  Serial.begin(115200);
  SettingsManager::instance().load();
  // Enable async TX buffer (non-blocking sends). Remove this line for pure synchronous TX.
  AsyncPacketBuffer::begin(Serial, 1, 1, 1200);

  // Start RX task
  xTaskCreatePinnedToCore(
    rxTask,
    "ProtoRX",
    4096,
    nullptr,
    1,
    nullptr,
    1
  );

  protoComm.sendDebug("System ready.");
}

void loop() {
  // main loop unused; logic runs in rxTask
}
