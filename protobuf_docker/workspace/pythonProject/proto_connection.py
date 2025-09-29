
from __future__ import annotations
import time
import random
import threading
from typing import Optional, Callable
from proto_serial_client import ProtoSerialClient
from log import logger

class ReconnectSupervisor:
    """
    Owns a ProtoRuntime and keeps a ProtoSerialClient connected.
    - Rebinds runtime.client on every successful (re)connect
    - Sends periodic keepalive pings while link is up
    - Closes and retries on read/parse errors or keepalive failure
    - Exponential backoff with jitter between attempts
    """

    def __init__(
        self,
        port: str,
        baudrate: int = 230400,
        keepalive_s: float = 3.0,
        idle_reset_s: float = 10.0,
        backoff_min: float = 0.5,
        backoff_max: float = 8.0,
        on_link_up: Optional[Callable[[ProtoSerialClient], None]] = None,
        on_link_down: Optional[Callable[[], None]] = None,
    ) -> None:
        self.port = port
        self.baudrate = baudrate
        self.keepalive_s = keepalive_s
        self.idle_reset_s = idle_reset_s
        self.backoff_min = backoff_min
        self.backoff_max = backoff_max

        self._client_lock = threading.RLock()
        self._client: Optional[ProtoSerialClient] = None

        self._stop = threading.Event()
        self._th = threading.Thread(target=self._run, name="reconnect-supervisor", daemon=True)

        self._on_link_up = on_link_up
        self._on_link_down = on_link_down

        # For status
        self._last_rx_monotonic = 0.0
        self._last_keepalive = 0.0

    # ---------------- API ----------------

    def start(self) -> None:
        self._stop.clear()
        if not self._th.is_alive():
            self._th = threading.Thread(target=self._run, name="reconnect-supervisor", daemon=True)
            self._th.start()

    def stop(self) -> None:
        self._stop.set()
        c = self._get_client()
        if c:
            try:
                c.stop()
            except Exception:
                pass
        self._bind_client(None)

    def is_connected(self) -> bool:
        c = self._get_client()
        return bool(c and getattr(c, "running", False))

    # ------------- internals -------------

    def _get_client(self) -> Optional[ProtoSerialClient]:
        with self._client_lock:
            return self._client

    def _bind_client(self, client: Optional[ProtoSerialClient]) -> None:
        with self._client_lock:
            self._client = client
            self.runtime.client = client  # <- single source of truth for GUI/commands

    def _on_ingest_wrapper(self, msg) -> None:
        # Called from client's listener thread
        self._last_rx_monotonic = time.monotonic()
        try:
            self.runtime.ingest(msg)
        except Exception as e:
            logger.exception(f"Ingest error: {e}")

    def _open_client(self) -> Optional[ProtoSerialClient]:
        try:
            c = ProtoSerialClient(self.port, self.baudrate)
            # Ensure client's listener calls us back
            # We *monkey patch* a tiny shim if ProtoSerialClient supports a callback
            if hasattr(c, "on_message"):
                c.on_message = self._on_ingest_wrapper  # type: ignore[attr-defined]
            # Start listener and verify alive
            c.start_listener()
            self._last_rx_monotonic = time.monotonic()
            logger.info(f"Link up: {self.port} @{self.baudrate}")
            return c
        except Exception as e:
            logger.error(f"Open client failed: {e}")
            return None

    def _close_client(self, c: Optional[ProtoSerialClient]) -> None:
        if not c:
            return
        try:
            c.stop()
        except Exception:
            pass
        logger.info("🔌 Link down")

    def _send_keepalive(self, c: ProtoSerialClient) -> bool:
        try:
            # Runtime exposes ping(); fall back to client if needed
            if hasattr(self.runtime, "ping"):
                self.runtime.ping()
            else:
                # Conservative fallback
                if hasattr(c, "send_ping"):
                    c.send_ping()
            return True
        except Exception as e:
            logger.warning(f"keepalive ping failed: {e}")
            return False

    def _run(self) -> None:
        delay = self.backoff_min
        while not self._stop.is_set():
            # 1) Try connecting
            c = self._open_client()
            if not c:
                # Backoff after open failure
                jitter = 0.2 * delay * (random.random() - 0.5)  # ±10%
                wait_s = max(self.backoff_min, min(self.backoff_max, delay + jitter))
                logger.info(f"Reconnect in {wait_s:.2f}s")
                time.sleep(wait_s)
                delay = min(self.backoff_max, max(self.backoff_min, delay * 2.0))
                continue

            # 2) Bind runtime + fire link_up callback
            self._bind_client(c)
            if self._on_link_up:
                try:
                    self._on_link_up(c)
                except Exception:
                    logger.exception("on_link_up callback failed")

            # 3) Connected loop
            delay = self.backoff_min
            self._last_keepalive = 0.0
            try:
                while not self._stop.is_set():
                    now = time.monotonic()

                    # detect idle RX (client listener should update _last_rx_monotonic via _on_ingest_wrapper)
                    idle = now - self._last_rx_monotonic
                    if idle > self.idle_reset_s:
                        logger.warning(f"RX idle for {idle:.1f}s → reconnect")
                        break

                    # periodic keepalive
                    if now - self._last_keepalive >= self.keepalive_s:
                        self._last_keepalive = now
                        if not self._send_keepalive(c):
                            # If keepalive fails once, try again on next tick;
                            # if it keeps failing, idle detector will break loop
                            pass

                    # If client has its own running flag, observe it
                    if not getattr(c, "running", True):
                        logger.warning("Client listener not running → reconnect")
                        break

                    time.sleep(0.2)

            finally:
                # 4) Link tear-down
                try:
                    if self._on_link_down:
                        try:
                            self._on_link_down()
                        except Exception:
                            logger.exception("on_link_down callback failed")
                finally:
                    self._close_client(c)
                    self._bind_client(None)

            # 5) Backoff before next attempt
            if self._stop.is_set():
                break
            jitter = 0.2 * delay * (random.random() - 0.5)  # ±10%
            wait_s = max(self.backoff_min, min(self.backoff_max, delay + jitter))
            logger.info(f"⏳ Reconnect in {wait_s:.2f}s")
            time.sleep(wait_s)
            delay = min(self.backoff_max, max(self.backoff_min, delay * 2.0))
