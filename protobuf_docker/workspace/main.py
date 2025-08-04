import time
from proto_serial_client import ProtoSerialClient

PORT = 'COM7'  # Passe an deinen Port an
BAUDRATE = 115200

def main():
    client = ProtoSerialClient(PORT, BAUDRATE)
    client.start_listener()

    print("🔌 Verbindung hergestellt. Eingaben:")
    print("[1] send_alive")
    print("[2] get_data")
    print("[3] toggle_mux")
    print("[q] quit")

    try:
        while True:
            cmd = input("👉 Befehl: ").strip().lower()
            if cmd == '1':
                client.send_alive()
            elif cmd == '2':
                client.send_get_data()
            elif cmd == '3':
                client.send_set_mux()
            elif cmd == 'q':
                break
            else:
                print("❓ Ungültige Eingabe")
            time.sleep(0.1)
    finally:
        client.stop()
        print("🚪 Verbindung geschlossen.")

if __name__ == "__main__":
    main()
