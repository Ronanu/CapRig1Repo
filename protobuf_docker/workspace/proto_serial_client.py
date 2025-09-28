import threading
import serial
import struct
import time
from messages_pb2 import ToEsp32, FromEsp32, SystemSettings

class ProtoSerialClient:
    def __init__(self, port, baudrate=115200, seq_start=1):
        self.port = port
        self.baudrate = baudrate
        self._seq = seq_start
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.running = False
        self.listener_thread = threading.Thread(target=self.listen, daemon=True)

    def _next_seq(self):
        s = self._seq
        self._seq = 1 if s >= 0xFFFFFFFF else s + 1
        return s

    def start_listener(self):
        self.running = True
        self.listener_thread.start()

    def stop(self):
        self.running = False
        if self.listener_thread.is_alive():
            self.listener_thread.join()
        self.ser.close()

    # ---------- framing ----------
    def _write_framed(self, payload: bytes):
        length = len(payload)
        if length > 128:
            raise ValueError("Message too long (>128 bytes)")
        header = struct.pack(">H", length)
        self.ser.write(header + payload)

    def _read_framed(self):
        header = self.ser.read(2)
        if len(header) != 2:
            return None
        (length,) = struct.unpack(">H", header)
        if length == 0 or length > 128:
            return None
        data = self.ser.read(length)
        if len(data) != length:
            return None
        return data

    # ---------- high-level send/recv ----------
    def send_message(self, msg: ToEsp32):
        self._write_framed(msg.SerializeToString())

    def read_message(self):
        data = self._read_framed()
        if not data:
            return None
        msg = FromEsp32()
        msg.ParseFromString(data)
        return msg

    # ---------- listener ----------
    def listen(self):
        while self.running:
            msg = self.read_message()
            if msg:
                self.handle_message(msg)
            time.sleep(0.01)

    def handle_message(self, msg: FromEsp32):
        print(f"  🧾 seq: {msg.seq}  🕒 timestamp: {msg.timestamp}")
        if msg.HasField("debug"):
            pass
            print(f"  🐞 Debug: {msg.debug.text}")
        elif msg.HasField("ack"):
            pass
        elif msg.HasField("error"):
            err = msg.error.error
            if err == "tx_queue_full":
                print("  ❌ ERROR: tx_queue_full (ESP32 TX-Queue ausgelastet)")
            else:
                print(f"  ❌ ERROR: {err}")
        elif msg.HasField("settings"):
            s = msg.settings
            print(f"  ⚙️ Settings → current_signal_selection_state={s.current_signal_selection_state}, action_state={s.action_state}")
        elif msg.HasField("sample"):
            smp = msg.sample
            print(f"  📊 Sample → sensor_id={smp.sensor_id}, value={smp.value:.3f}, checksum=0x{smp.checksum:08X}")
        else:
            print("  ❓ Unbekannte Antwort")

    # ---------- convenience commands ----------
    def send_ping(self, seq=None):
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.ping.SetInParent()
        self.send_message(msg)

    def send_get_settings(self, seq=None):
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.get_settings.SetInParent()
        self.send_message(msg)

    def send_set_settings(self, current_signal_selection_state: bool, action_state: int, seq=None):
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.set_settings.settings.current_signal_selection_state = bool(current_signal_selection_state)
        # constrain 0..3 on client side as well
        msg.set_settings.settings.action_state = max(0, min(3, int(action_state)))
        self.send_message(msg)

    def send_set_mux(self, channel: int, seq=None):
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.set_mux.channel = int(channel)
        self.send_message(msg)

    def send_ping_1000x(self):
        print("🚀 Starte 1000x ping Test...")
        start = time.time()
        for i in range(1000):
            self.send_ping(seq=i+1)
            time.sleep(0.0005)
        dt = time.time() - start
        time.sleep(3)  # Warte auf ausstehende Antworten
        print(f"✅ 1000 Nachrichten in {dt:.3f}s → {(1000/dt):.2f} Hz")
