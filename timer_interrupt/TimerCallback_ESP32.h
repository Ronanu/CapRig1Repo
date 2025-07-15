#ifndef TIMER_CALLBACK_ESP32_H
#define TIMER_CALLBACK_ESP32_H

/**
 * @brief Einfacher Hardware-Timer für ESP32 (v3.x kompatibel).
 * 
 * Verwendung:
 * void IRAM_ATTR onMyTimer() { myTimer.handleInterrupt(); }
 * TimerCallback myTimer;
 * myTimer.begin(1000.0f, onMyTimer);
 * myTimer.attachCallback(myFunction, context);
 * myTimer.start();
 */

#include <Arduino.h>

class TimerCallback {
public:
    TimerCallback();
    ~TimerCallback();
    
    bool begin(float frequency, void (*interruptHandler)());
    bool start();
    bool stop();
    void attachCallback(void (*callback)(void*), void* context = nullptr);
    void handleInterrupt();
    
private:
    hw_timer_t* _timer;
    void (*_userCallback)(void*);
    void* _userContext;
    bool _running;
};

#endif
