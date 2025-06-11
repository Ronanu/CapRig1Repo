#ifndef TIMER_INTERRUPT_HANDLER_H
#define TIMER_INTERRUPT_HANDLER_H

#include <Arduino.h>
#include "hal_data.h"

class TimerInterruptHandler {
public:
    TimerInterruptHandler(gpt_instance_ctrl_t* ctrl, gpt_cfg_t* cfg, uint32_t interval_us);
    void begin();
    void attachCallback(void (*callback)());

private:
    gpt_instance_ctrl_t* _ctrl;
    gpt_cfg_t* _cfg;
    uint32_t _interval_us;
    static void (*_userCallback)();
    static void internalCallback(timer_callback_args_t* args);
};

#endif