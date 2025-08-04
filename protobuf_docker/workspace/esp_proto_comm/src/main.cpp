#include <Arduino.h>
#include "ProtobufComm.hpp"
#include "SampleManager.hpp"
#include "messages.pb.h"

SampleManager sampleManager;
ProtobufComm protoComm(Serial, sampleManager);

void setup() {
  Serial.begin(115200);
}

void sendAck(const char* msg) {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = millis();
  response.which_payload = FromEsp32_ack_tag;
  strncpy(response.ack.message, msg, sizeof(response.ack.message) - 1);
  response.ack.message[sizeof(response.ack.message) - 1] = '\0';
  protoComm.send(response);
}

void sendError(const char* msg) {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = millis();
  response.which_payload = FromEsp32_error_tag;
  strncpy(response.error.error, msg, sizeof(response.error.error) - 1);
  response.error.error[sizeof(response.error.error) - 1] = '\0';
  protoComm.send(response);
}

void TaskReceive(void* pvParameters) {
  for (;;) {
    ToEsp32 msg = ToEsp32_init_zero;
    if (protoComm.receive(msg)) {
      switch (msg.which_payload) {
        case ToEsp32_request_status_tag:
          sendAck("ACK");
          break;
        case ToEsp32_set_pwm_tag:
          sendError("SetPwm not implemented");
          break;
        case ToEsp32_control_output_tag:
          sendError("ControlOutput not implemented");
          break;
        case ToEsp32_set_config_tag:
          sendError("SetConfig not implemented");
          break;
        default:
          sendError("Unknown command");
          break;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void loop() {
  TaskReceive(nullptr);
}
