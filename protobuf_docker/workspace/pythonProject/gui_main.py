
import tkinter as tk
from tkinter import ttk, messagebox
import threading
import time

from log import logger
from proto_connection import ReconnectSupervisor

PORT = "COM4"   # anpassen
BAUDRATE = 230400

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("ESP32 Proto (minimal)")
        self.geometry("540x360")

        # Connection supervisor
        self.conn = ReconnectSupervisor(PORT, BAUDRATE)
        self.conn.start()

        # UI
        self.lbl_status = ttk.Label(self, text="Status: connecting...", font=("Segoe UI", 12))
        self.lbl_status.pack(pady=8)

        frm = ttk.Frame(self)
        frm.pack(pady=8)

        self.btn_ping = ttk.Button(frm, text="Ping", command=self._cmd_ping)
        self.btn_ping.grid(row=0, column=0, padx=6)

        self.btn_get = ttk.Button(frm, text="Get Settings", command=self._cmd_get)
        self.btn_get.grid(row=0, column=1, padx=6)

        self.btn_set = ttk.Button(frm, text="Set Settings", command=self._cmd_set)
        self.btn_set.grid(row=0, column=2, padx=6)

        self.txt = tk.Text(self, height=12)
        self.txt.pack(fill="both", expand=True, padx=8, pady=8)

        # periodic UI update
        self.after(250, self._tick)

    # ---------- commands ----------
    def _guard(self):
        if not self.conn.is_connected():
            raise RuntimeError("Not connected")
        return True

    def _cmd_ping(self):
        try:
            self._guard()
            self.runtime.ping()
            self._log("Ping sent")
        except Exception as e:
            self._err(e)

    def _cmd_get(self):
        try:
            self._guard()
            st = self.runtime.get_settings()
            self._log(f"Settings: {st}")
        except Exception as e:
            self._err(e)

    def _cmd_set(self):
        try:
            self._guard()
            # Minimal demo values; adjust to your schema
            st = self.runtime.set_settings(current_signal_selection_state=False, action_state=1)
            self._log(f"Set Settings result: {st}")
        except Exception as e:
            self._err(e)

    # ---------- ui helpers ----------
    def _tick(self):
        connected = self.conn.is_connected()
        self.lbl_status.configure(text=f"Status: {'connected' if connected else 'reconnecting...'}")
        self.after(500, self._tick)

    def _log(self, s: str):
        self.txt.insert("end", s + "\n")
        self.txt.see("end")
        logger.info(s)

    def _err(self, e: Exception):
        msg = f"{type(e).__name__}: {e}"
        self._log("ERROR " + msg)
        messagebox.showerror("Error", msg)

if __name__ == "__main__":
    App().mainloop()
