#include "TimerCallback_ESP32.h"

TimerCallback::TimerCallback() : _timer(nullptr), _userCallback(nullptr), _userContext(nullptr), _running(false) {}

TimerCallback::~TimerCallback() {
    stop();
    if (_timer) {
        timerEnd(_timer);
    }
}

bool TimerCallback::begin(float frequency, void (*interruptHandler)()) {
    if (_timer) {
        Serial.println("Timer bereits initialisiert!");
        return false;  
    }
    
    Serial.printf("Erstelle Timer für %f Hz\n", frequency);
    
    // Berechne die Periode in Mikrosekunden
    uint32_t period_us = (uint32_t)(1000000.0f / frequency);
    Serial.printf("Periode: %lu Mikrosekunden\n", period_us);
    
    // ESP32 v3.x: Timer mit 1MHz Basis erstellen
    _timer = timerBegin(1000000);  // 1MHz Basis
    if (!_timer) {
        Serial.println("timerBegin fehlgeschlagen");
        return false;
    }
    Serial.println("Timer erstellt!");
    
    // Alarm für gewünschte Frequenz setzen
    Serial.printf("Setze Alarm auf %lu us\n", period_us);
    timerAlarm(_timer, period_us, true, 0);  // Timer, Periode, Reload, Count
    Serial.println("Alarm gesetzt!");
    
    // Interrupt anhängen
    Serial.println("Hänge Interrupt an");
    timerAttachInterrupt(_timer, interruptHandler);
    Serial.println("Interrupt angehängt!");
    
    return true;
}

bool TimerCallback::start() {
    if (!_timer) {
        Serial.println("Kein Timer zum Starten!");
        return false;
    }
    if (_running) {
        Serial.println("Timer läuft bereits!");
        return false;
    }
    Serial.println("Starte Timer...");
    timerStart(_timer);
    _running = true;
    Serial.println("Timer gestartet!");
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
    if (_userCallback) {
        _userCallback(_userContext);
    }
}
