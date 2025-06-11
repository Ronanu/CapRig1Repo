#include "TimerInterruptHandler.h"

void (*TimerInterruptHandler::_userCallback)() = nullptr;

TimerInterruptHandler::TimerInterruptHandler(gpt_instance_ctrl_t* ctrl, gpt_cfg_t* cfg, uint32_t interval_us)
    : _ctrl(ctrl), _cfg(cfg), _interval_us(interval_us) {}

void TimerInterruptHandler::attachCallback(void (*callback)()) {
    _userCallback = callback;
}

void TimerInterruptHandler::begin() {
    R_GPT_Open(_ctrl, _cfg);
    R_GPT_PeriodSet(_ctrl, _interval_us * 240); // 240 MHz → 240 Takte pro µs
    R_GPT_CallbackSet(_ctrl, internalCallback, nullptr, nullptr);
    R_GPT_Enable(_ctrl);
    R_GPT_Start(_ctrl);
}

void TimerInterruptHandler::internalCallback(timer_callback_args_t* args) {
    if (_userCallback) {
        _userCallback();
    }
}