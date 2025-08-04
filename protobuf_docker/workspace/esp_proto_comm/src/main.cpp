#include <Arduino.h>
#include "SampleManager.hpp"
#include "ProtobufComm.hpp"
#include "CommandHandler.hpp"

// Globale Instanzen
CommandHandler commandHandler(sampleManager, protoComm);
SampleManager sampleManager;
ProtobufComm protoComm(Serial, sampleManager);

// Timer und Tasks
hw_timer_t* spiTimer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onSpiTimer() {
  static bool toggle = false;
  toggle = !toggle;
  float dummyValue = toggle ? 1.23f : 4.56f;
  sampleManager.addSample(toggle ? 1 : 0, dummyValue); // Sensor ID aus toggle ableiten
}

void TaskSend(void* pvParameters) {
  for (;;) {
    protoComm.sendStatus();
    vTaskDelay(pdMS_TO_TICKS(1)); // 1 kHz
  }
}

void TaskReceive(void* pvParameters) {
  for (;;) {
    protoComm.receiveAndHandle();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup() {
  protoComm.setDispatcher(&commandHandler);
  Serial.begin(115200);
  delay(100);

  // Timer für SPI
  spiTimer = timerBegin(0, 80, true); // 80 MHz / 80 = 1 MHz Ticks
  timerAttachInterrupt(spiTimer, &onSpiTimer, true);
  timerAlarmWrite(spiTimer, 200, true); // 5 kHz => 200 µs
  timerAlarmEnable(spiTimer);

  // Tasks starten
  xTaskCreatePinnedToCore(TaskReceive, "Receive", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskSend, "Send", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // leer
}
