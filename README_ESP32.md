# Timer Interrupt Library - ESP32 Port

Dieses Projekt enthält eine Timer-Interrupt-Bibliothek, die ursprünglich für den Arduino Uno R4 WiFi entwickelt wurde und nun auch für ESP32 Dev Kits portiert wurde.

## Dateien

### Original (Arduino Uno R4 WiFi)
- `TimerCallback.h` / `TimerCallback.cpp` - FspTimer-basierte Implementierung
- `timer_interrupt.ino` - Hauptprogramm für Uno R4 WiFi

### ESP32 Port
- `TimerCallback_ESP32.h` / `TimerCallback_ESP32.cpp` - ESP32-Hardware-Timer-Implementierung
- `timer_interrupt_ESP32.ino` - Hauptprogramm für ESP32 Dev Kit

## Hauptunterschiede ESP32 vs. Arduino Uno R4 WiFi

### Hardware-Timer
- **Uno R4 WiFi**: Nutzt Renesas RA4M1 GPT-Timer mit FspTimer-API
- **ESP32**: Nutzt ESP32-Hardware-Timer (4 Timer verfügbar: 0-3)

### Timer-Verfügbarkeit
- **Uno R4 WiFi**: 7 GPT-Kanäle, einige für PWM reserviert
- **ESP32**: 4 Hardware-Timer, Timer 0 oft von WiFi-Stack verwendet

### Interrupt-Handling
- **Uno R4 WiFi**: Standard Arduino-Interrupts (`noInterrupts()`/`interrupts()`)
- **ESP32**: FreeRTOS-basiert (`portDISABLE_INTERRUPTS()`/`portENABLE_INTERRUPTS()`)

### Performance
- **Uno R4 WiFi**: Bis zu ~10 kHz stabil getestet
- **ESP32**: Theoretisch bis zu mehreren MHz, praktisch bis ~50 kHz empfohlen

## Installation und Verwendung

### Für ESP32 Dev Kit:

1. Arduino IDE mit ESP32-Support installieren
2. ESP32 Dev Kit Board auswählen
3. Dateien `TimerCallback_ESP32.h`, `TimerCallback_ESP32.cpp` und `timer_interrupt_ESP32.ino` verwenden
4. Code kompilieren und hochladen

### Wichtige Hinweise für ESP32:

1. **Timer-Zuweisung**: Timer 1-3 werden bevorzugt, Timer 0 nur als Fallback
2. **Interrupt-Funktionen**: Sollten mit `IRAM_ATTR` markiert werden für bessere Performance
3. **Serial-Output**: Kurze Verzögerung nach `Serial.begin()` empfohlen
4. **Memory**: ESP32 hat mehr RAM, kann also auch höhere Frequenzen handhaben

## Beispiel-Output

```
ESP32 Timer-Test gestartet...
Alle Timer gestartet.
Erwartete Werte pro Sekunde: 10kHz=10000, 1kHz=1000, 100Hz=100
Laufzeit: 1.0s | 10kHz: 10000 | 1kHz: 1000 | 100Hz: 100 | Effizienz: 100.0% / 100.0% / 100.0%
Laufzeit: 2.0s | 10kHz: 10000 | 1kHz: 1000 | 100Hz: 100 | Effizienz: 100.0% / 100.0% / 100.0%
...
*** Timer werden gestoppt (5s erreicht) ***
Laufzeit: 6.0s | 10kHz: 0 | 1kHz: 0 | 100Hz: 0 | Effizienz: 0.0% / 0.0% / 0.0%
...
*** Timer werden neu gestartet (10s erreicht) ***
```

## Troubleshooting

### Timer-Initialisierung fehlgeschlagen
- Prüfen Sie, ob genügend Hardware-Timer verfügbar sind
- WiFi-Verwendung kann Timer 0 blockieren

### Niedrige Effizienz bei hohen Frequenzen
- Interrupt-Funktionen sollten sehr kurz sein
- Keine `Serial.print()` oder `delay()` in Interrupt-Routinen
- Bei >20kHz: Prüfen Sie CPU-Last und andere laufende Tasks

### ESP32-spezifische Probleme
- ESP32 läuft mit 240 MHz (vs. 48 MHz beim Uno R4 WiFi)
- Watchdog-Timer können bei sehr hoher Interrupt-Last auslösen
- FreeRTOS-Task-Scheduler kann Timing beeinflussen

## Lizenz

Entwickelt für HESS4MRA Testing.
Greetings, Felix.
