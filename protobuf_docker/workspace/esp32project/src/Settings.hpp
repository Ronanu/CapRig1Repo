#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "messages_nanopb.pb.h" // for SystemSettings

struct Settings {
  bool     current_signal_selection_state = false;
  uint32_t action_state = 0; // 0..3
};

class SettingsManager {
public:
  static SettingsManager& instance();

  Settings get();
  void set(const Settings& s);

  // Map to/from protobuf
  void toProto(SystemSettings& out);
  void fromProto(SystemSettings& in);

  // Persistence
  void load();
  void saveDebounced();
  void tick(); // call periodically to perform debounced save

private:
  SettingsManager();
  void saveNow();

  Settings s_;
  SemaphoreHandle_t mtx_;
  unsigned long last_change_us_ = 0;
  static constexpr unsigned long SAVE_DEBOUNCE_US = 3000000; // 3s
};

#endif // SETTINGS_HPP
