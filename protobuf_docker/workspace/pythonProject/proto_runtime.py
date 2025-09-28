
"""
High‑level Protobuf Runtime for ESP32 link.
- RequestManager: tracks command/response lifecycles with ACK + total timeouts
- SampleLossDetector: detects lost/duplicate/out‑of‑order samples via monotonically increasing seq

Keep ProtoSerialClient slim: it only sends/receives frames.
Integration:
    runtime = ProtoRuntime(client=None)
    client.set_runtime(runtime)
    # set client in runtime on link-up: runtime.client = client
    runtime.ingest(msg)  # called by listener
"""
from __future__ import annotations
import threading
import time
from dataclasses import dataclass, field
from enum import Enum, auto
from typing import Dict, Optional, Set

from log import logger  # project logger (colored console + rotating file)

# --- Proto message imports ---
try:
    from messages_pb2 import FromEsp32, SystemSettings  # type: ignore
except Exception:  # pragma: no cover
    FromEsp32 = object  # shim for type hints
    SystemSettings = object


class RespKind(Enum):
    ACK = auto()
    INFO = auto()
    SETTINGS = auto()
    ERROR = auto()
    DEBUG = auto()
    SAMPLE = auto()
    OTHER = auto()


@dataclass
class RequestState:
    seq: int
    expect: Set[RespKind]
    ack_timeout_s: float
    total_timeout_s: float
    created_ts: float = field(default_factory=time.monotonic)
    got: Set[RespKind] = field(default_factory=set)
    info_message: Optional[str] = None
    error_text: Optional[str] = None
    settings: Optional[SystemSettings] = None
    ack_seen: bool = False
    event: threading.Event = field(default_factory=threading.Event)


class RequestManager:
    """Track outstanding requests by seq and fulfill expectations with timeouts."""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._pending: Dict[int, RequestState] = {}

    def register(self, seq: int, expect: Set[RespKind], ack_timeout_s: float, total_timeout_s: float) -> None:
        with self._lock:
            if seq in self._pending:
                logger.warning(f"register(): seq {seq} already pending — overwriting")
            self._pending[seq] = RequestState(seq, set(expect), ack_timeout_s, total_timeout_s)
            logger.debug(f"Registered request seq={seq} expect={[k.name for k in expect]}")

    def notify(self, msg: FromEsp32) -> None:
        kind = self._classify(msg)
        seq = int(getattr(msg, 'seq', 0))
        with self._lock:
            st = self._pending.get(seq)
        if st is None:
            # stray or unsolicited (e.g. samples). Not an error.
            logger.debug(f"notify(): unsolicited {kind.name} seq={seq}")
            return

        changed = False
        if kind == RespKind.ACK:
            st.ack_seen = True
            st.got.add(RespKind.ACK)
            changed = True
        elif kind == RespKind.INFO:
            st.info_message = getattr(msg.info, 'message', None)
            st.got.add(RespKind.INFO)
            changed = True
        elif kind == RespKind.SETTINGS:
            st.settings = msg.settings
            st.got.add(RespKind.SETTINGS)
            changed = True
        elif kind == RespKind.ERROR:
            st.error_text = getattr(msg.error, 'error', None)
            st.got.add(RespKind.ERROR)
            changed = True
        elif kind == RespKind.DEBUG:
            # Treat DEBUG as INFO fallback when INFO not present in expect
            txt = getattr(msg.debug, 'text', None)
            if RespKind.INFO in st.expect and st.info_message is None:
                st.info_message = txt
                st.got.add(RespKind.INFO)
                changed = True

        if changed:
            logger.debug(f"notify(): seq={seq} got={[k.name for k in st.got]}")
            # If we have everything, release waiter
            if st.expect.issubset(st.got):
                st.event.set()

    def wait(self, seq: int) -> RequestState:
        """Block for ACK then for all expected messages; return final state.
        Timeouts are encoded in the returned state via missing flags.
        """
        with self._lock:
            st = self._pending.get(seq)
        if st is None:
            raise RuntimeError(f"wait(): seq {seq} not registered")

        # Phase 1: ACK deadline
        ack_deadline = st.created_ts + st.ack_timeout_s
        while time.monotonic() < ack_deadline and not st.ack_seen:
            if st.event.wait(timeout=0.02):
                if st.ack_seen:
                    break
        if not st.ack_seen:
            logger.warning(f"ACK timeout for seq={seq}")
            # continue to wait for total, might still complete late

        # Phase 2: total deadline
        total_deadline = st.created_ts + st.total_timeout_s
        while time.monotonic() < total_deadline:
            if st.expect.issubset(st.got):
                break
            st.event.wait(timeout=0.02)

        with self._lock:
            # Pop to avoid leaks
            self._pending.pop(seq, None)
        return st

    @staticmethod
    def _classify(msg: FromEsp32) -> RespKind:
        # Use HasField when available
        try:
            if msg.HasField("ack"):
                return RespKind.ACK
            if msg.HasField("info"):
                return RespKind.INFO
            if msg.HasField("settings"):
                return RespKind.SETTINGS
            if msg.HasField("error"):
                return RespKind.ERROR
            if msg.HasField("debug"):
                return RespKind.DEBUG
            if msg.HasField("sample"):
                return RespKind.SAMPLE
        except Exception:
            pass
        # Fallback classification
        for name in ("ack","info","settings","error","debug","sample"):
            if hasattr(msg, name):
                return {
                    "ack": RespKind.ACK,
                    "info": RespKind.INFO,
                    "settings": RespKind.SETTINGS,
                    "error": RespKind.ERROR,
                    "debug": RespKind.DEBUG,
                    "sample": RespKind.SAMPLE,
                }[name]
        return RespKind.OTHER


class SampleLossDetector:
    """Monitors monotonically increasing 32‑bit seq of samples and detects losses/dupes.
    Call update(seq, ts_us) for each sample.
    """

    def __init__(self) -> None:
        self.last_seq: Optional[int] = None
        self.last_ts_us: Optional[int] = None
        self.lost_total = 0
        self.dup_total = 0
        self.reset_count = 0
        self._last_report = time.monotonic()
        self._samples_since_report = 0

    def update(self, seq: int, ts_us: int) -> None:
        if self.last_seq is None:
            self.last_seq, self.last_ts_us = seq, ts_us
            return

        # compute forward modular delta (uint32)
        d = (seq - self.last_seq) & 0xFFFFFFFF
        if d == 0:
            self.dup_total += 1
        elif 1 <= d < (1 << 31):
            if d > 1:
                self.lost_total += (d - 1)
        else:
            # negative jump (OOO or device reset). Heuristic: treat as reset if big back jump
            self.reset_count += 1
        self.last_seq, self.last_ts_us = seq, ts_us

        self._samples_since_report += 1
        now = time.monotonic()
        if now - self._last_report >= 1.0:  # once per second
            logger.info(
                f"samples: +{self._samples_since_report}/s, lost_total={self.lost_total}, dup_total={self.dup_total}, resets={self.reset_count}"
            )
            self._samples_since_report = 0
            self._last_report = now


class ProtoRuntime:
    """Glue: wire RequestManager + SampleLossDetector to a transport (ProtoSerialClient)."""

    def __init__(self, client) -> None:
        self.client = client
        self.rm = RequestManager()
        self.loss = SampleLossDetector()

    # --- inbound path ---
    def ingest(self, msg: FromEsp32) -> None:
        kind = RequestManager._classify(msg)
        if kind == RespKind.SAMPLE:
            smp = msg.sample
            # seq holds the sample counter per ESP spec; timestamp is the measurement time
            seq = int(getattr(msg, 'seq', 0))
            ts_us = int(getattr(msg, 'timestamp', 0))
            self.loss.update(seq, ts_us)
            logger.debug(f"sample(seq={seq}, value={getattr(smp,'value',0.0):.3f}, ts={ts_us})")
            return
        # non-sample messages go to RequestManager
        self.rm.notify(msg)

    # --- high-level requests (blocking convenience) ---
    def ping(self, ack_timeout=0.2, total_timeout=1.0):
        if not self.client:
            raise RuntimeError("No client bound to runtime")
        seq = self.client.send_ping(return_seq=True)
        self.rm.register(seq, {RespKind.ACK, RespKind.INFO}, ack_timeout, total_timeout)
        st = self.rm.wait(seq)
        ok = st.ack_seen and ({RespKind.ACK, RespKind.INFO} - st.got == set())
        if not ok:
            missing = {RespKind.ACK, RespKind.INFO} - st.got
            logger.warning(f"ping() incomplete: missing={[m.name for m in missing]}")
        else:
            logger.info(f"ping() ok: info='{st.info_message}'")
        return st

    def get_settings(self, ack_timeout=0.2, total_timeout=1.0):
        if not self.client:
            raise RuntimeError("No client bound to runtime")
        seq = self.client.send_get_settings(return_seq=True)
        self.rm.register(seq, {RespKind.ACK, RespKind.SETTINGS}, ack_timeout, total_timeout)
        st = self.rm.wait(seq)
        ok = st.ack_seen and ({RespKind.ACK, RespKind.SETTINGS} - st.got == set())
        if not ok:
            missing = {RespKind.ACK, RespKind.SETTINGS} - st.got
            logger.warning(f"get_settings() incomplete: missing={[m.name for m in missing]}")
        else:
            logger.info("get_settings() ok")
        return st

    def set_settings(self, new_settings: SystemSettings, ack_timeout=0.2, total_timeout=1.0):
        if not self.client:
            raise RuntimeError("No client bound to runtime")
        seq = self.client.send_set_settings(new_settings, return_seq=True)
        # INFO is optional; SETTINGS is required to confirm
        self.rm.register(seq, {RespKind.ACK, RespKind.SETTINGS}, ack_timeout, total_timeout)
        st = self.rm.wait(seq)
        ok = st.ack_seen and ({RespKind.ACK, RespKind.SETTINGS} - st.got == set())
        if not ok:
            missing = {RespKind.ACK, RespKind.SETTINGS} - st.got
            logger.warning(f"set_settings() incomplete: missing={[m.name for m in missing]}")
        else:
            if st.info_message:
                logger.info(f"set_settings() ok: info='{st.info_message}'")
            else:
                logger.info("set_settings() ok")
        return st
