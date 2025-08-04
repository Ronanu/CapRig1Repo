#include <Arduino.h>
#include "SampleManager.hpp"
#include "ProtobufComm.hpp"
#include "CommandHandler.hpp"

// Globale Instanzen
SampleManager sampleManager;
ProtobufComm protoComm(Serial, sampleManager);
CommandHandler commandHandler(sampleManager, Serial);

// Timer und Tasks
hw_timer_t* spiTimer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onSpiTimer() {
  static bool toggle = false;
  toggle = !toggle;
  float dummyValue = toggle ? 1.23f : 4.56f;
  sampleManager.addSample(toggle ? 1 : 0, dummyValue); // Sensor ID aus toggle ableiten
}

void TaskReceive(void* pvParameters) {
  for (;;) {
    ToEsp32 msg = ToEsp32_init_zero;
    if (protoComm.receive(msg)) {
      commandHandler.dispatch(msg);
    }
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
}

void loop() {
  // leer
}
