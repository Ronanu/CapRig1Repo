#pragma once
#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "messages_nanopb.pb.h"

struct Settings {
  bool     current_signal_selection_state = false; // vorher vorhanden
  uint32_t action_state = 0;                   // vorher vorhanden
  bool     sample_streaming_enabled = true;       // NEU: true = Streaming dauerhaft aktiv
};

class SettingsManager {
public:
  static SettingsManager& instance();

  Settings get();
  void set(const Settings& s);

  void load();
  void saveDebounced();
  void tick();

  // Mapping zu/von Protobuf (Nanopb)
  void toProto(SystemSettings& out);
  void fromProto(const SystemSettings& in);

private:
  SettingsManager();
  void requestSave(); // intern für Debounce

  SemaphoreHandle_t mtx_;
  Settings          settings_;

  uint32_t last_save_request_us_;
  bool     pending_save_;
};
