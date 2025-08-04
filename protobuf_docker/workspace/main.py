import time
from proto_serial_client import ProtoSerialClient

PORT = 'COM7'  # Passe an deinen Port an
BAUDRATE = 115200

def main():
    client = ProtoSerialClient(PORT, BAUDRATE)
    client.start_listener()

    print("🔌 Verbindung hergestellt. Verfügbare Befehle:")
    print("┌─────┬──────────────────────────────────┐")
    print("│ [1] │ send_alive                       │")
    print("│ [2] │ get_data                         │")
    print("│ [3] │ toggle_mux                       │")
    print("│ [4] │ 1000x get_data (Frequenz-Test)   │")
    print("│ [q] │ quit                             │")
    print("└─────┴──────────────────────────────────┘")

    try:
        while True:
            cmd = input("\n👉 Befehl eingeben: ").strip().lower()
            
            if cmd == '1':
                print("📤 Sende alive...")
                client.send_alive()
            elif cmd == '2':
                print("📤 Sende get_data...")
                client.send_get_data()
            elif cmd == '3':
                print("📤 Sende toggle_mux...")
                client.send_set_mux()
            elif cmd == '4':
                print("📤 Starte 1000x get_data Test...")
                client.send_get_data_1000x()
            elif cmd == 'q':
                print("👋 Beende Programm...")
                break
            else:
                print("❌ Ungültige Eingabe! Bitte [1], [2], [3], [4] oder [q] eingeben.")
            
            time.sleep(0.1)
    finally:
        print("\n🔄 Schließe Verbindung...")
        client.stop()
        print("✅ Verbindung erfolgreich geschlossen.")

if __name__ == "__main__":
    main()
