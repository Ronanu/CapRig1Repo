#ifndef TIMER_CALLBACK_ESP32_H
#define TIMER_CALLBACK_ESP32_H

/**
 * @class TimerCallback
 * @brief Einfacher Hardware-Timer für ESP32 (v3.x kompatibel).
 */

#include <Arduino.h>

class TimerCallback {
public:
    TimerCallback();
    ~TimerCallback();
    bool begin(float frequency, void (*interruptHandler)());
    bool start();
    bool stop();
    void attachCallback(void (*callback)(void*), void* context);
    void handleInterrupt();
    
private:
    hw_timer_t* _timer;
    float _frequency;
    void (*_userCallback)(void*);
    void* _userContext;
    bool _running;
};

#endif
