#ifndef TIMER_CALLBACK_H
#define TIMER_CALLBACK_H

#include <FspTimer.h>

class TimerCallback {
public:
    TimerCallback();
    bool begin(float frequency);
    void attachCallback(void (*callback)(void*), void* context);

private:
    FspTimer _timer;

    void (*_userCallback)(void*) = nullptr;
    void* _userContext = nullptr;

    static void trampoline(timer_callback_args_t* args);
};

#endif