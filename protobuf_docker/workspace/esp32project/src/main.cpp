#include <Arduino.h>
#include <SPI.h>
#include "ADC78H89.h"
#include "ProtobufComm.hpp"
#include "Settings.hpp"
#include "messages_nanopb.pb.h"
#include "AsyncPacketBuffer.hpp"

// -------------------- Serial / tasks --------------------
constexpr unsigned long SERIAL_BAUDRATE = 500000;
constexpr TickType_t RX_TASK_DELAY = pdMS_TO_TICKS(1);

// -------------------- ADC config ------------------------
// Fixed by request: CS pin = 15, channel = 6
#define ADC_CS_PIN          15
#define ADC_CHANNEL_VOLTAGE    6   // valid: 0..6 (AIN1..AIN7)
#define ADC_CHANNEL_CURRENT    5   // valid: 0..6 (AIN1..AIN7)
#define SAMPLE_HZ          10   // adjust if needed

// -------------------- Globals ---------------------------
ProtobufComm protoComm(Serial);
static ADC78H89 adc(ADC_CS_PIN);

static hw_timer_t*        g_sampleTimer   = nullptr;
static SemaphoreHandle_t  g_sampleSem     = nullptr;
static volatile uint32_t  g_sampleCounter = 0;   // monotonically increasing, used as seq for samples

// -------------------- Proto helpers ---------------------
// Ack is intentionally EMPTY now.
static void sendAck(uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;                 // keep correlation if host expects it
  res.timestamp = micros();      // time of ack creation
  res.which_response = FromEsp32_ack_tag;
  // no payload fields (ack is empty)
  protoComm.send(res);
}

// INFO message carries human-readable text
static void sendInfo(const char* msg, uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_info_tag;
  strncpy(res.response.info.message, msg, sizeof(res.response.info.message) - 1);
  protoComm.send(res);
}

static void sendError(const char* msg, uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_error_tag;
  strncpy(res.response.error.error, msg, sizeof(res.response.error.error) - 1);
  protoComm.send(res);
}

static void sendSettings(uint32_t seq) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = seq;
  res.timestamp = micros();
  res.which_response = FromEsp32_settings_tag;

  // Falls SettingsManager bereits das neue Feld kennt, gerne direkt via toProto().
  // Danach setzen wir das Streaming-Flag explizit auf true (dauerhaft aktiv).
  SettingsManager::instance().toProto(res.response.settings);
  res.response.settings.sample_streaming_enabled = true;

  protoComm.send(res);
}

// Timestamp wird am Messpunkt ermittelt und als Parameter übergeben.
static void sendSample(uint32_t sensor_id, float value, uint32_t sample_seq, uint32_t timestamp_us) {
  FromEsp32 res = FromEsp32_init_zero;
  res.seq = sample_seq;            // use sample counter as seq for samples
  res.timestamp = timestamp_us;    // measurement timestamp (from sampling site)
  res.which_response = FromEsp32_sample_tag;
  res.response.sample.sensor_id = sensor_id;
  res.response.sample.value = value;
  union { float f; uint32_t u; } conv = { value };
  res.response.sample.checksum = sensor_id ^ conv.u;
  protoComm.send(res);
}

// -------------------- Timer ISR -------------------------
void IRAM_ATTR onSampleTimer() {
  BaseType_t hp = pdFALSE;
  if (g_sampleSem) xSemaphoreGiveFromISR(g_sampleSem, &hp);
  if (hp) portYIELD_FROM_ISR();
}

// -------------------- Sampling Task (Core 0) ------------
void sampleTask(void* /*pv*/) {
  // Initialize ADC/SPI in task context (never in ISR)
  adc.begin();

  for (;;) {
    if (false){ //(xSemaphoreTake(g_sampleSem, portMAX_DELAY) == pdTRUE) {
      // fixed channel by request
      uint16_t raw = adc.readChannel(ADC_CHANNEL_VOLTAGE);
      // timestamp measured immediately after the conversion returned
      uint32_t ts = micros();
      uint32_t seq = g_sampleCounter++;
      // Send raw code as float; convert to volts on the host if desired
      sendSample(/*sensor_id*/ 1, static_cast<float>(raw), seq, ts);
      raw = adc.readChannel(ADC_CHANNEL_CURRENT);
      sendSample(/*sensor_id*/ 2, static_cast<float>(raw), seq, ts);
    }
  }
}

// -------------------- RX Task (Core 1) ------------------
void rxTask(void* /*pv*/) {
  for (;;) {
    ToEsp32 cmd = ToEsp32_init_zero;
    if (protoComm.receive(cmd)) {
      const uint32_t seq = cmd.seq;
      switch (cmd.which_command) {
        case ToEsp32_ping_tag:
          sendAck(seq);
          break;

        case ToEsp32_get_settings_tag:
          sendSettings(seq);
          break;

        case ToEsp32_set_settings_tag: {
          const SystemSettings& in = cmd.command.set_settings.settings;
          SettingsManager::instance().fromProto(in);
          SettingsManager::instance().saveDebounced();
          sendSettings(seq);
          break;
        }

        default:
          sendError("unknown command", seq);
          break;
      }
    } else {
      vTaskDelay(RX_TASK_DELAY);
    }

    SettingsManager::instance().tick();
  }
}

// -------------------- Setup / Loop ----------------------
void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  SettingsManager::instance().load();

  // Asynchroner TX-Puffer für non-blocking Proto-Sends
  AsyncPacketBuffer::begin(Serial, /*qLen*/ 1, /*taskPrio*/ 1, /*stackWords*/ 1200);

  // RX task auf Core 1 (Kommunikation)
  xTaskCreatePinnedToCore(
    rxTask,
    "ProtoRX",
    4096,
    nullptr,
    10,
    nullptr,
    1
  );

  // Sampling-Sync
  g_sampleSem = xSemaphoreCreateBinary();

  // Sampling task auf Core 0 (Erfassung)
  xTaskCreatePinnedToCore(
    sampleTask,
    "SampleTask",
    4096,
    nullptr,
    1,      
    nullptr,
    0
  );

  // HW-Timer @ SAMPLE_HZ
  g_sampleTimer = timerBegin(/*timer*/0, /*prescaler*/80, /*countUp*/true); // 1 tick = 1 us (APB 80 MHz)
  timerAttachInterrupt(g_sampleTimer, &onSampleTimer, /*edge*/true);
  const uint32_t ticks = 1000000UL / SAMPLE_HZ; // microseconds per sample
  timerAlarmWrite(g_sampleTimer, ticks, /*autoReload*/true);
  timerAlarmEnable(g_sampleTimer);

  protoComm.sendDebug("System ready. ADC sampling active (streaming_enabled = true).");
}

void loop() {
  // All logic is in tasks
}
