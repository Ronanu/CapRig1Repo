#include "TimerCallback_ESP32.h"

TimerCallback::TimerCallback() 
    : _timer(nullptr), _userCallback(nullptr), _userContext(nullptr), _running(false) {}

TimerCallback::~TimerCallback() {
    stop();
    if (_timer) timerEnd(_timer);
}

bool TimerCallback::begin(float frequency, void (*interruptHandler)()) {
    if (_timer) return false;  // Bereits initialisiert
    
    // Timer mit 1MHz Basis erstellen
    _timer = timerBegin(1000000);
    if (!_timer) return false;
    
    // Alarm für gewünschte Frequenz setzen
    uint32_t period_us = (uint32_t)(1000000.0f / frequency);
    timerAlarm(_timer, period_us, true, 0);
    
    // Interrupt anhängen
    timerAttachInterrupt(_timer, interruptHandler);
    return true;
}

bool TimerCallback::start() {
    if (!_timer || _running) return false;
    timerStart(_timer);
    _running = true;
    return true;
}

bool TimerCallback::stop() {
    if (!_timer || !_running) return false;
    timerStop(_timer);
    _running = false;
    return true;
}

void TimerCallback::attachCallback(void (*callback)(void*), void* context) {
    _userCallback = callback;
    _userContext = context;
}

void TimerCallback::handleInterrupt() {
    if (_userCallback) _userCallback(_userContext);
}
