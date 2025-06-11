#include "TimerCallback.h"

TimerCallback::TimerCallback() {}

bool TimerCallback::begin(float frequency) {
    uint8_t timer_type = GPT_TIMER;

    int8_t tindex = FspTimer::get_available_timer(timer_type);
    if (tindex < 0) {
        tindex = FspTimer::get_available_timer(timer_type, true);
        if (tindex < 0) return false;
    }

    FspTimer::force_use_of_pwm_reserved_timer();

    // Übergabe des Kontexts hier
    if (!_timer.begin(TIMER_MODE_PERIODIC, timer_type, tindex, frequency, 0.0f, trampoline, this)) return false;
    if (!_timer.setup_overflow_irq()) return false;
    if (!_timer.open()) return false;
    if (!_timer.start()) return false;

    return true;
}

void TimerCallback::attachCallback(void (*callback)(void*), void* context) {
    _userCallback = callback;
    _userContext = context;
}

void TimerCallback::trampoline(timer_callback_args_t* args) {
    if (args && args->p_context) {
        // korrekter Cast mit const_cast
        TimerCallback* self = const_cast<TimerCallback*>(static_cast<const TimerCallback*>(args->p_context));
        if (self->_userCallback) {
            self->_userCallback(self->_userContext);
        }
    }
}