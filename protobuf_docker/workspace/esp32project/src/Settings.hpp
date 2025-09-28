
#pragma once

#include <Arduino.h>
#include "messages_nanopb.pb.h"  // for SystemSettings
#include <stdint.h>

struct Settings {
  bool     current_signal_selection_state = false;
  uint32_t action_state = 0; // 0..3
};

class SettingsManager {
public:
  static SettingsManager& instance();

  Settings get();
  void set(const Settings& s);

  // Load from NVS (Preferences)
  void load();

  // Request a debounced save (actual write happens in tick())
  void saveDebounced();

  // Must be called periodically (e.g., each loop iteration or task cycle)
  void tick();

  // Mapping between internal Settings and protobuf struct
  void toProto(SystemSettings& out);
  void fromProto(const SystemSettings& in);

private:
  SettingsManager();
  void requestSave();

  Settings settings_;
  SemaphoreHandle_t mtx_;

  uint32_t last_save_request_us_;
  bool pending_save_;
};
