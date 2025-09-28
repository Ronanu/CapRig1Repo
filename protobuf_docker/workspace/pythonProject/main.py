import time
from proto_serial_client import ProtoSerialClient

PORT = 'COM4'   # Anpassen
BAUDRATE = 230400

def main():
    client = ProtoSerialClient(PORT, BAUDRATE)
    client.start_listener()

    print("🔌 Verbindung hergestellt. Verfügbare Befehle:")
    print("┌─────┬────────────────────────────────────────────┐")
    print("│ [1] │ ping                                       │")
    print("│ [2] │ get_settings                               │")
    print("│ [3] │ set_settings (fragt 2 Werte ab)            │")
    print("│ [4] │ set_mux (fragt Kanal ab)                   │")
    print("│ [5] │ 1000x ping (Frequenz-Test)                 │")
    print("│ [q] │ quit                                       │")
    print("└─────┴────────────────────────────────────────────┘")

    try:
        while True:
            cmd = input("\n👉 Befehl eingeben: ").strip().lower()

            if cmd == '1':
                print("📤 Sende ping...")
                client.send_ping()
            elif cmd == '2':
                print("📤 Sende get_settings...")
                client.send_get_settings()
            elif cmd == '3':
                print("📤 Sende set_settings...")
                try:
                    css = input("  ⚙️ current_signal_selection_state [0/1]: ").strip()
                    css_bool = (css == '1' or css.lower() in ('true','t','y','yes','ja'))
                    act = int(input("  ⚙️ action_state [0..3]: ").strip())
                except Exception as e:
                    print(f"  ❌ Eingabe ungültig: {e}")
                    continue
                client.send_set_settings(css_bool, act)
            elif cmd == '4':
                print("📤 Sende set_mux...")
                try:
                    ch = int(input("  🔀 Kanal (uint): ").strip())
                except Exception as e:
                    print(f"  ❌ Eingabe ungültig: {e}")
                    continue
                client.send_set_mux(ch)
            elif cmd == '5':
                print("📤 Starte 1000x ping Test...")
                client.send_ping_1000x()
            elif cmd == 'q':
                print("👋 Beende Programm...")
                break
            else:
                print("❌ Ungültige Eingabe! Bitte [1]..[5] oder [q] eingeben.")

            time.sleep(0.1)
    finally:
        print("\n🔄 Schließe Verbindung...")
        client.stop()
        print("✅ Verbindung erfolgreich geschlossen.")

if __name__ == "__main__":
    main()
