#ifndef TIMER_CALLBACK_H
#define TIMER_CALLBACK_H

/**
 * @class TimerCallback
 * @brief Verwaltet einen GPT-Timer auf dem Arduino Uno R4 WiFi mit FSP-Unterstützung.
 *
 * Diese Klasse kapselt die Verwendung eines Hardware-Timers (General Purpose Timer) 
 * mithilfe der FspTimer-API. Ein Benutzer kann eine Callback-Funktion mit Kontext 
 * registrieren, die mit einer festen Frequenz durch Timer-Interrupts aufgerufen wird.
 *
 * Merkmale:
 * - Nutzt den GPT-Timer (General Purpose Timer) der Renesas RA4M1 MCU.
 * - Unterstützt Frequenzen typischerweise bis zu 10 kHz stabil (abhängig von Interrupt-Dauer).
 * - Verwendet FspTimer::get_available_timer(), um verfügbare Timer automatisch zuzuweisen.
 * - Übergabe von benutzerdefinierter Callback-Funktion mit optionalem Kontext (z. B. Sensorinstanz).
 *
 * Einschränkungen:
 * - Anzahl nutzbarer Timer ist hardwareabhängig (RA4M1 bietet 7 GPT-Kanäle, manche für PWM reserviert).
 * - Wenn der Interrupt-Code länger dauert als das Timer-Intervall, kann der nächste Interrupt 
 *   verloren gehen (kein Interrupt-Stapeln, da keine Priorisierung/Queueing im GPT).
 * - Callback-Funktionen sollten **kurz und effizient** sein – keine Serial.prints oder Delays im Interrupt.
 *
 * Verwendung:
 * 
 * TimerCallback myTimer;
 * myTimer.begin(1000.0f);  // 1 kHz Timer
 * myTimer.attachCallback(myFunction, contextPointer);
 *
 * Siehe auch: FspTimer-Dokumentation im Arduino Core für Renesas.
 */

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