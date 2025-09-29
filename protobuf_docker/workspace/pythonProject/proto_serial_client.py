import threading
import struct
import time
from typing import Optional, Callable, Tuple

import serial
from messages_pb2 import ToEsp32, FromEsp32
from log import logger


class ProtoSerialClient:
    """
    Minimal-Client mit eingebautem Reconnect:
    - start()/stop() steuern einen Supervisor-Thread
    - Listener-Thread liest Frames und ruft optional on_message(msg)
    - Keepalive via ping() in festen Intervallen
    - Idle-Detector: wenn RX zu lange still → reconnect
    """

    def __init__(
        self,
        port: str,
        baudrate: int = 115200,
        keepalive_s: float = 3.0,
        idle_reset_s: float = 10.0,
        backoff: Tuple[float, float] = (0.5, 8.0),
    ) -> None:
        self.port = port
        self.baudrate = baudrate
        self.keepalive_s = keepalive_s
        self.idle_reset_s = idle_reset_s
        self.backoff_min, self.backoff_max = backoff

        # Zustände
        self._ser: Optional[serial.Serial] = None
        self._stop = threading.Event()
        self._tx_lock = threading.RLock()
        self._listener_thread: Optional[threading.Thread] = None
        self._supervisor_thread: Optional[threading.Thread] = None
        self.running = False  # true, solange der Listener aktiv ist
        self._last_rx_monotonic = 0.0
        self._last_keepalive = 0.0
        self._seq = 1

        # Callbacks
        self.on_message: Optional[Callable[[FromEsp32], None]] = None
        self.on_link_up: Optional[Callable[[], None]] = None
        self.on_link_down: Optional[Callable[[], None]] = None

    # ---------- Public API ----------

    def start(self) -> None:
        if self._supervisor_thread and self._supervisor_thread.is_alive():
            return
        self._stop.clear()
        self._supervisor_thread = threading.Thread(
            target=self._run_supervisor, name="serial-supervisor", daemon=True
        )
        self._supervisor_thread.start()

    def stop(self) -> None:
        self._stop.set()
        self._close_serial()

    def is_connected(self) -> bool:
        return bool(self._ser and self._ser.is_open and self.running)

    # Convenience Befehle (nutzen framing & ToEsp32)
    def send_ping(self, seq: Optional[int] = None) -> None:
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.ping.SetInParent()
        self._send_message(msg)

    def send_get_settings(self, seq: Optional[int] = None) -> None:
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.get_settings.SetInParent()
        self._send_message(msg)

    def send_set_settings(self, current_signal_selection_state: bool, action_state: int, seq: Optional[int] = None) -> None:
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.set_settings.settings.current_signal_selection_state = bool(current_signal_selection_state)
        msg.set_settings.settings.action_state = max(0, min(3, int(action_state)))
        self._send_message(msg)

    def send_set_mux(self, channel: int, seq: Optional[int] = None) -> None:
        msg = ToEsp32()
        msg.seq = self._next_seq() if seq is None else int(seq)
        msg.set_mux.channel = int(channel)
        self._send_message(msg)

    def send_ping_1000x(self) -> None:
        logger.info("Starte 1000x ping Test…")
        start = time.time()
        for i in range(1000):
            self.send_ping(seq=i + 1)
            time.sleep(0.0005)
        dt = time.time() - start
        time.sleep(3.0)
        logger.info(f"1000 Nachrichten in {dt:.3f}s → {(1000/dt):.2f} Hz")

    # ---------- Internals ----------

    def _next_seq(self) -> int:
        s = self._seq
        self._seq = 1 if s >= 0xFFFFFFFF else s + 1
        return s

    def _open_serial(self) -> bool:
        try:
            self._ser = serial.Serial(self.port, self.baudrate, timeout=0.1)
            self.running = True
            self._last_rx_monotonic = time.monotonic()
            self._listener_thread = threading.Thread(target=self._listen_loop, name="serial-listener", daemon=True)
            self._listener_thread.start()
            logger.info(f"Link up: {self.port} @{self.baudrate}")
            if self.on_link_up:
                try:
                    self.on_link_up()
                except Exception:
                    logger.exception("on_link_up callback error")
            return True
        except Exception as e:
            logger.error(f"Port öffnen fehlgeschlagen: {e}")
            self._ser = None
            self.running = False
            return False

    def _close_serial(self) -> None:
        lt = self._listener_thread
        self.running = False
        if lt and lt.is_alive():
            lt.join(timeout=1.0)
        self._listener_thread = None
        if self._ser:
            try:
                self._ser.close()
            except Exception:
                pass
            self._ser = None
        logger.info("🔌 Link down")
        if self.on_link_down:
            try:
                self.on_link_down()
            except Exception:
                logger.exception("on_link_down callback error")

    def _run_supervisor(self) -> None:
        delay = self.backoff_min
        while not self._stop.is_set():
            # Verbinden versuchen
            if not self._open_serial():
                # Backoff
                wait_s = max(self.backoff_min, min(self.backoff_max, delay))
                logger.info(f"Reconnect in {wait_s:.2f}s")
                time.sleep(wait_s)
                delay = min(self.backoff_max, max(self.backoff_min, delay * 2.0))
                continue

            # Verbunden → überwachen
            delay = self.backoff_min
            self._last_keepalive = 0.0
            try:
                while not self._stop.is_set():
                    now = time.monotonic()

                    # Idle-Überwachung
                    if now - self._last_rx_monotonic > self.idle_reset_s:
                        logger.warning("RX idle reconnect")
                        break

                    # Keepalive
                    if now - self._last_keepalive >= self.keepalive_s:
                        self._last_keepalive = now
                        try:
                            self.send_ping()
                        except Exception as e:
                            logger.warning(f"Keepalive fehlgeschlagen: {e}")
                            # Nächste Runde prüft Idle/Running ohnehin

                    # Listener noch aktiv?
                    if not self.running:
                        logger.warning("Listener gestoppt reconnect")
                        break

                    time.sleep(0.2)
            finally:
                self._close_serial()

            # Backoff vor nächstem Versuch
            if self._stop.is_set():
                break
            wait_s = max(self.backoff_min, min(self.backoff_max, delay))
            logger.info(f"Reconnect in {wait_s:.2f}s")
            time.sleep(wait_s)
            delay = min(self.backoff_max, max(self.backoff_min, delay * 2.0))

    # ---------- Framing/IO ----------

    def _send_message(self, msg: ToEsp32) -> None:
        if not self.is_connected():
            raise RuntimeError("Nicht verbunden")
        payload = msg.SerializeToString()
        if len(payload) > 128:
            logger.error("Message zu lang (>128 Bytes)")
        header = struct.pack(">H", len(payload))
        with self._tx_lock:
            self._ser.write(header + payload)  # type: ignore[union-attr]

    def _read_framed(self) -> Optional[bytes]:
        ser = self._ser
        if not ser:
            return None
        header = ser.read(2)
        if len(header) != 2:
            return None
        (length,) = struct.unpack(">H", header)
        if length == 0 or length > 128:
            return None
        data = ser.read(length)
        if len(data) != length:
            return None
        return data

    def _listen_loop(self) -> None:
        try:
            while self._ser and self.running:
                try:
                    data = self._read_framed()
                    if not data:
                        time.sleep(0.01)
                        continue
                    msg = FromEsp32()
                    msg.ParseFromString(data)
                    self._last_rx_monotonic = time.monotonic()
                    self._handle_message(msg)
                    if self.on_message:
                        try:
                            self.on_message(msg)
                        except Exception:
                            logger.exception("on_message callback error")
                except Exception as e:
                    logger.error(f"Listener-Fehler: {e}")
                    # Listener wird beendet, Supervisor reconnectet
                    break
        finally:
            self.running = False

    # ---------- Standard-Logging (optional) ----------

    def _handle_message(self, msg: FromEsp32) -> None:
        if msg.HasField("ack"):
            logger.info("ACK")
        elif msg.HasField("debug"):
            logger.debug(f"Debug: {msg.debug.text}")
        elif msg.HasField("info"):
            logger.info(f"Info: {msg.info.text}")
        elif msg.HasField("error"):
            err = msg.error.error
            if err == "tx_queue_full":
                logger.warning("ERROR: tx_queue_full")
            else:
                logger.error(f"ERROR: {err}")
        elif msg.HasField("settings"):
            s = msg.settings
            logger.info(
                f"Settings  current_signal_selection_state={s.current_signal_selection_state}, "
                f"action_state={s.action_state}, timestamp={msg.timestamp}"
            )
        elif msg.HasField("sample"):
            smp = msg.sample
            logger.debug(
                f"Sample → sensor_id={smp.sensor_id}, value={smp.value:.3f}, "
                f"checksum=0x{smp.checksum:08X}, timestamp={msg.timestamp}"
            )
        else:
            logger.info("Unbekannte Antwort")
