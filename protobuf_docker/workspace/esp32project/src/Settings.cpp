
#include "Settings.hpp"
#include <Preferences.h>

static const char* kNs = "appcfg";
static const char* kKeySigSel = "sigsel";
static const char* kKeyAction = "action";

SettingsManager& SettingsManager::instance() {
  static SettingsManager inst;
  return inst;
}

SettingsManager::SettingsManager() {
  mtx_ = xSemaphoreCreateMutex();
  last_save_request_us_ = 0;
  pending_save_ = false;
  // Defaults
  settings_.current_signal_selection_state = false;
  settings_.action_state = 0;
}

Settings SettingsManager::get() {
  Settings copy;
  xSemaphoreTake(mtx_, portMAX_DELAY);
  copy = settings_;
  xSemaphoreGive(mtx_);
  return copy;
}

void SettingsManager::set(const Settings& s) {
  xSemaphoreTake(mtx_, portMAX_DELAY);
  settings_ = s;
  xSemaphoreGive(mtx_);
}

void SettingsManager::load() {
  Preferences pref;
  pref.begin(kNs, /*readOnly=*/true);
  Settings s;
  s.current_signal_selection_state = pref.getBool(kKeySigSel, false);
  s.action_state = pref.getUInt(kKeyAction, 0);
  pref.end();
  set(s);
}

void SettingsManager::requestSave() {
  pending_save_ = true;
  last_save_request_us_ = micros();
}

void SettingsManager::saveDebounced() {
  // schedule a save ~100ms later via tick()
  requestSave();
}

void SettingsManager::tick() {
  if (!pending_save_) return;
  const uint32_t now = micros();
  // 100ms debounce
  if ((uint32_t)(now - last_save_request_us_) >= 100000) {
    pending_save_ = false;
    // persist current settings
    Settings s = get();
    Preferences pref;
    pref.begin(kNs, /*readOnly=*/false);
    pref.putBool(kKeySigSel, s.current_signal_selection_state);
    // clamp 0..3 for safety
    uint32_t act = s.action_state;
    if (act > 3) act = 3;
    pref.putUInt(kKeyAction, act);
    pref.end();
  }
}

void SettingsManager::toProto(SystemSettings& out) {
  Settings s = get();
  out.current_signal_selection_state = s.current_signal_selection_state;
  out.action_state = s.action_state;
}

void SettingsManager::fromProto(const SystemSettings& in) {
  Settings s = get();
  s.current_signal_selection_state = in.current_signal_selection_state;
  s.action_state = in.action_state > 3 ? 3 : in.action_state;
  set(s);
}
