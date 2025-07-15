#include "TimerCallback_ESP32.h"

// Static member initialization
bool TimerCallback::timerUsed[4] = {false, false, false, false};

TimerCallback::TimerCallback() 
    : _timer(nullptr), _timerNum(255), _frequency(0), _initialized(false), 
      _running(false), _userCallback(nullptr), _userContext(nullptr) {
}

TimerCallback::~TimerCallback() {
    if (_initialized) {
        stop();
        if (_timer) {
            timerEnd(_timer);
            _timer = nullptr;
        }
        if (_timerNum < 4) {
            timerUsed[_timerNum] = false;
        }
    }
}

int8_t TimerCallback::getAvailableTimer() {
    // Timer 0 oft von WiFi verwendet, also erst ab Timer 1 versuchen
    for (int i = 1; i < 4; i++) {
        if (!timerUsed[i]) {
            return i;
        }
    }
    // Falls Timer 1-3 belegt, versuche Timer 0
    if (!timerUsed[0]) {
        return 0;
    }
    return -1;  // Keine Timer verfügbar
}

bool TimerCallback::begin(float frequency, void (*interruptHandler)()) {
    if (_initialized) {
        return false;  // Bereits initialisiert
    }
    
    _frequency = frequency;
    
    // Verfügbaren Timer finden
    int8_t timerNum = getAvailableTimer();
    if (timerNum < 0) {
        Serial.println("Fehler: Kein Hardware-Timer verfügbar!");
        return false;
    }
    
    _timerNum = timerNum;
    timerUsed[_timerNum] = true;
    
    // Timer initialisieren
    // ESP32 Timer läuft mit 80 MHz, wir nutzen Prescaler 80 für 1 MHz Basis
    // Dann ist die Alarm-Wert = 1000000 / frequency
    _timer = timerBegin(_timerNum, 80, true);  // Timer, Prescaler, Count-Up
    if (!_timer) {
        Serial.println("Fehler: Timer konnte nicht initialisiert werden!");
        timerUsed[_timerNum] = false;
        return false;
    }
    
    // Berechne Alarm-Wert für gewünschte Frequenz
    uint32_t alarmValue = (uint32_t)(1000000.0f / frequency);  // 1MHz / frequency
    
    // Timer konfigurieren (aber noch nicht starten)
    timerAlarmWrite(_timer, alarmValue, true);  // Timer, Alarm-Wert, Auto-Reload
    
    // Externe Interrupt-Handler setzen
    timerAttachInterrupt(_timer, interruptHandler, true);  // Edge-triggered
    
    _initialized = true;
    return true;
}

bool TimerCallback::start() {
    if (!_initialized || _running) {
        return false;
    }
    
    timerAlarmEnable(_timer);
    _running = true;
    return true;
}

bool TimerCallback::stop() {
    if (!_initialized || !_running) {
        return false;
    }
    
    timerAlarmDisable(_timer);
    _running = false;
    return true;
}

void TimerCallback::attachCallback(void (*callback)(void*), void* context) {
    _userCallback = callback;
    _userContext = context;
}

void TimerCallback::handleInterrupt() {
    if (_userCallback) {
        _userCallback(_userContext);
    }
}
