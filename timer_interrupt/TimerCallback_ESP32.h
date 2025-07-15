#ifndef TIMER_CALLBACK_ESP32_H
#define TIMER_CALLBACK_ESP32_H

/**
 * @class TimerCallback
 * @brief Verwaltet einen Hardware-Timer auf dem ESP32 Dev Kit.
 *
 * Diese Klasse kapselt die Verwendung eines Hardware-Timers mithilfe der ESP32-Timer-API.
 * Ein Benutzer kann eine Callback-Funktion mit Kontext registrieren, die mit einer 
 * festen Frequenz durch Timer-Interrupts aufgerufen wird.
 *
 * Merkmale:
 * - Nutzt die ESP32 Hardware-Timer (4 Timer verfügbar: Timer 0-3).
 * - Unterstützt Frequenzen bis zu mehreren MHz (abhängig von Interrupt-Dauer).
 * - Automatische Timer-Zuweisung mit get_available_timer().
 * - Übergabe von benutzerdefinierter Callback-Funktion mit optionalem Kontext.
 *
 * Einschränkungen:
 * - ESP32 hat 4 Hardware-Timer (Timer 0-3), die zwischen allen Anwendungen geteilt werden.
 * - Timer 0 wird oft vom WiFi-Stack verwendet.
 * - Callback-Funktionen sollten **kurz und effizient** sein – keine Serial.prints oder Delays im Interrupt.
 * - Bei sehr hohen Frequenzen (>50kHz) kann es zu Timing-Problemen kommen.
 *
 * Verwendung:
 * 
 * TimerCallback myTimer;
 * myTimer.begin(1000.0f);  // 1 kHz Timer
 * myTimer.attachCallback(myFunction, contextPointer);
 * myTimer.start();
 *
 * Kompatibel mit ESP32 Arduino Core.
 */

#include <Arduino.h>

class TimerCallback {
public:
    TimerCallback();
    ~TimerCallback();
    bool begin(float frequency);                 // Initialisiert, aber startet NICHT den Timer
    bool start();                                // Startet den Timer (Interrupts aktiv)
    bool stop();                                 // Stoppt den Timer (Interrupts inaktiv)
    void attachCallback(void (*callback)(void*), void* context);
    
private:
    hw_timer_t* _timer;
    uint8_t _timerNum;
    float _frequency;
    bool _initialized;
    bool _running;
    
    void (*_userCallback)(void*);
    void* _userContext;
    
    static void IRAM_ATTR onTimer(TimerCallback* instance);
    static bool timerUsed[4];  // Track which timers are in use
    static int8_t getAvailableTimer();
};

#endif
