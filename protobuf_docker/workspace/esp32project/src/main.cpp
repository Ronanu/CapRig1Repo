#include <Arduino.h>
#include "ProtobufComm.hpp"
#include "messages_nanopb.pb.h"

constexpr TickType_t RECEIVE_TASK_DELAY = pdMS_TO_TICKS(1);
constexpr gpio_num_t MUX_PIN = GPIO_NUM_13;

ProtobufComm protoComm(Serial);

void sendAck(const char* msg) {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = micros();
  response.which_response = FromEsp32_ack_tag;
  strncpy(response.response.ack.message, msg, sizeof(response.response.ack.message) - 1);
  response.response.ack.message[sizeof(response.response.ack.message) - 1] = '\0';
  protoComm.send(response);
}

void sendError(const char* msg) {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = micros();
  response.which_response = FromEsp32_error_tag;
  strncpy(response.response.error.error, msg, sizeof(response.response.error.error) - 1);
  response.response.error.error[sizeof(response.response.error.error) - 1] = '\0';
  protoComm.send(response);
}

void sendDummySample() {
  FromEsp32 response = FromEsp32_init_zero;
  response.timestamp = micros();
  response.which_response = FromEsp32_sample_tag;
  response.response.sample.sensor_id = 1;
  response.response.sample.value = 42.0f;
  response.response.sample.checksum = protoComm.calculateChecksum(1, 42.0f);
  protoComm.send(response);
}

void toggleMuxPin() {
  digitalWrite(MUX_PIN, !digitalRead(MUX_PIN));
}

void TaskReceive(void* pvParameters) {
  for (;;) {
    ToEsp32 msg = ToEsp32_init_zero;
    if (protoComm.receive(msg)) {
      protoComm.sendDebug("Received message");

      switch (msg.which_command) {
        case ToEsp32_alive_tag:
          sendAck("ESP32 is alive");
          break;

        case ToEsp32_get_data_tag:
          sendDummySample();
          break;

        case ToEsp32_set_mux_tag:
          toggleMuxPin();
          sendAck("MUX toggled");
          break;

        default:
          sendError("Unknown command received");
          break;
      }
    }
    vTaskDelay(RECEIVE_TASK_DELAY);
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);  // USB-Verbindung abwarten (optional)

  pinMode(MUX_PIN, OUTPUT);
  digitalWrite(MUX_PIN, LOW);

  xTaskCreatePinnedToCore(
    TaskReceive,
    "ProtoReceive",
    4096,
    nullptr,
    1,
    nullptr,
    1  // Core 1
  );

  protoComm.sendDebug("System ready.");
}

void loop() {
  // keine Hauptlogik hier
}
