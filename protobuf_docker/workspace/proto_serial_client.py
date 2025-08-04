import threading
import serial
import struct
import time
from messages_pb2 import ToEsp32, FromEsp32

class ProtoSerialClient:
    def __init__(self, port, baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.running = False
        self.listener_thread = threading.Thread(target=self.listen, daemon=True)

    def start_listener(self):
        self.running = True
        self.listener_thread.start()

    def stop(self):
        self.running = False
        if self.listener_thread.is_alive():
            self.listener_thread.join()
        self.ser.close()

    def send_message(self, msg: ToEsp32):
        data = msg.SerializeToString()
        length = len(data)
        if length > 128:
            raise ValueError("Message too long")
        header = struct.pack('>H', length)
        self.ser.write(header + data)

    def read_message(self):
        header = self.ser.read(2)
        if len(header) < 2:
            return None
        length = struct.unpack('>H', header)[0]
        if length > 128:
            return None
        data = self.ser.read(length)
        if len(data) != length:
            return None
        msg = FromEsp32()
        msg.ParseFromString(data)
        return msg

    def listen(self):
        while self.running:
            msg = self.read_message()
            if msg:
                self.handle_message(msg)
            time.sleep(0.01)

    def handle_message(self, msg: FromEsp32):
        print("📥 Nachricht empfangen:")
        print(f"  🕒 Zeitstempel: {msg.timestamp}")
        if msg.HasField("debug"):
            print(f"  🐞 Debug: {msg.debug.text}")
        elif msg.HasField("ack"):
            print(f"  ✅ ACK: {msg.ack.message}")
        elif msg.HasField("error"):
            print(f"  ❌ ERROR: {msg.error.error}")
        elif msg.HasField("sample"):
            pass
            #print(f"  📊 Sample → Sensor: {msg.sample.sensor_id}, Wert: {msg.sample.value}, Checksumme: {msg.sample.checksum}")
        else:
            print("  ❓ Unbekannte Antwort")

    def send_alive(self):
        msg = ToEsp32()
        msg.alive.SetInParent()
        self.send_message(msg)

    def send_get_data(self):
        msg = ToEsp32()
        msg.get_data.SetInParent()
        self.send_message(msg)

    def send_set_mux(self):
        msg = ToEsp32()
        msg.set_mux.toggle = True  # oder False, wenn nötig
        self.send_message(msg)

    def send_get_data_1000x(self):
        """Sendet 1000 mal get_data und misst die Sendefrequenz"""
        print("🚀 Starte 1000x get_data Test...")
        
        start_time = time.time()
        
        for i in range(1000):
            msg = ToEsp32()
            msg.get_data.SetInParent()
            self.send_message(msg)
            
            # Fortschritt alle 100 Nachrichten anzeigen
            if (i + 1) % 100 == 0:
                elapsed = time.time() - start_time
                current_freq = (i + 1) / elapsed
                print(f"  📈 {i + 1}/1000 gesendet - Aktuelle Frequenz: {current_freq:.2f} Hz")
        
        end_time = time.time()
        total_time = end_time - start_time
        frequency = 1000 / total_time
        
        print(f"\n✅ Test abgeschlossen:")
        print(f"  📊 1000 Nachrichten in {total_time:.3f} Sekunden gesendet")
        print(f"  🎯 Durchschnittliche Sendefrequenz: {frequency:.2f} Hz")
        print(f"  ⏱️ Zeit pro Nachricht: {total_time/1000*1000:.3f} ms")

