import serial
import struct
import time
from messages_pb2 import ToEsp32, FromEsp32

PORT = 'COM7'  # dein ESP32 ist hier angeschlossen
BAUDRATE = 115200

def send_message(ser, msg_obj):
    data = msg_obj.SerializeToString()
    length = len(data)
    if length > 128:
        raise ValueError("Message too long")
    header = struct.pack('>H', length)
    ser.write(header + data)

def read_message(ser):
    header = ser.read(2)
    if len(header) < 2:
        return None
    length = struct.unpack('>H', header)[0]
    if length > 128:
        return None
    data = ser.read(length)
    if len(data) != length:
        return None
    msg = FromEsp32()
    msg.ParseFromString(data)
    return msg

def main():
    with serial.Serial(PORT, BAUDRATE, timeout=0.1) as ser:
        print("⏳ Warte auf ESP32...")

        # Nachricht vorbereiten
        to_esp = ToEsp32()
        to_esp.timestamp = int(time.time() * 1000)
        to_esp.command = "get_status"
        to_esp.payload = ""
        to_esp.hash = 0

        send_message(ser, to_esp)
        print("📤 Nachricht gesendet. Warte auf Antwort...")

        msg = read_message(ser)
        if msg:
            print("✅ Antwort empfangen:")
            print(f"  🕒 Zeitstempel: {msg.timestamp}")
            print(f"  🌡️ Temperatur: {msg.temperature:.1f}°C")
            print(f"  💧 Feuchtigkeit: {msg.humidity:.1f}%")
            print(f"  📄 Status: {msg.status}")
            print(f"  🔐 Hash: {msg.hash}")
        else:
            print("❌ Keine gültige Antwort erhalten.")

if __name__ == "__main__":
    main()
