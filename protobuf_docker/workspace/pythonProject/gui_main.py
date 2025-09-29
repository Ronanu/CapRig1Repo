import tkinter as tk
from tkinter import ttk, messagebox

from log import logger
from proto_serial_client import ProtoSerialClient

PORT = "COM4"   # anpassen
BAUDRATE = 230400

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("ESP32 Proto (minimal)")
        self.geometry("560x380")

        # Client direkt nutzen (kein ReconnectSupervisor mehr)
        self.client = ProtoSerialClient(
            PORT,
            BAUDRATE,
            keepalive_s=3.0,
            idle_reset_s=10.0,
            backoff=(0.5, 8.0),
        )
        self.client.on_message = self._on_message
        self.client.start()

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

        self.after(250, self._tick)

    # ---------- commands ----------
    def _guard(self):
        if not self.client.is_connected():
            raise RuntimeError("Nicht verbunden")
        return True

    def _cmd_ping(self):
        try:
            self._guard()
            self.client.send_ping()
            self._log("Ping gesendet")
        except Exception as e:
            self._err(e)

    def _cmd_get(self):
        try:
            self._guard()
            self.client.send_get_settings()
            self._log("GetSettings angefordert")
        except Exception as e:
            self._err(e)

    def _cmd_set(self):
        try:
            self._guard()
            # Demo-Werte – an dein Schema anpassen
            self.client.send_set_settings(current_signal_selection_state=False, action_state=1)
            self._log("SetSettings gesendet (state=1)")
        except Exception as e:
            self._err(e)

    # ---------- callbacks & ui helpers ----------
    def _on_message(self, msg):
        # kommt aus Listener-Thread → in UI-Thread hoppen
        def _append():
            if msg.HasField("ack"):
                self._log("ACK")
            elif msg.HasField("settings"):
                s = msg.settings
                self._log(
                    f"Settings: current_signal_selection_state={s.current_signal_selection_state}, "
                    f"action_state={s.action_state}, ts={msg.timestamp}"
                )
            elif msg.HasField("info"):
                self._log(f"Info: {msg.info.text}")
            elif msg.HasField("error"):
                self._log(f"ERROR: {msg.error.error}")
            elif msg.HasField("debug"):
                self._log(f"Debug: {msg.debug.text}")
            elif msg.HasField("sample"):
                smp = msg.sample
                # self._log(f"Sample: id={smp.sensor_id}, val={smp.value:.3f}")
            else:
                self._log("Unbekannte Antwort")
        self.after(0, _append)

    def _tick(self):
        connected = self.client.is_connected()
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
1