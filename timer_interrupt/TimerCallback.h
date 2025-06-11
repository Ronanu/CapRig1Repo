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
    static void trampoline(timer_callback_args_t* args);

    static void (*_userCallback)(void*);
    static void* _userContext;
};

#endif