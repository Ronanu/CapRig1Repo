import json
import threading
import tkinter as tk
from tkinter.scrolledtext import ScrolledText

import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class GuiNode(Node):
    def __init__(self, ui_cb):
        super().__init__('gui_node')
        self.pub_cmd = self.create_publisher(String, '/gui/cmd', 10)
        self.sub_dummy = self.create_subscription(String, '/esp/dummy', self.on_dummy, 10)
        self.sub_echo = self.create_subscription(String, '/gui/echo', self.on_echo, 10)
        self.ui_cb = ui_cb

    def on_dummy(self, msg: String):
        try:
            data = json.loads(msg.data)
        except Exception:
            data = {"raw": msg.data}
        self.ui_cb("DUMMY", data)

    def on_echo(self, msg: String):
        try:
            data = json.loads(msg.data)
        except Exception:
            data = {"raw": msg.data}
        self.ui_cb("ECHO", data)

    def send_cmd(self, text: str):
        m = String()
        m.data = text
        self.pub_cmd.publish(m)


def start_ros(ui_cb):
    rclpy.init(args=None)
    node = GuiNode(ui_cb)
    t = threading.Thread(target=rclpy.spin, args=(node,), daemon=True)
    t.start()
    return node


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Ros2Rig - Tkinter GUI (in container)")
        self.geometry("860x520")

        top = tk.Frame(self); top.pack(fill=tk.X, padx=8, pady=8)
        tk.Button(top, text="PING", width=10, command=lambda: self.node.send_cmd("PING")).pack(side=tk.LEFT, padx=4)
        tk.Button(top, text="LED_ON", width=10, command=lambda: self.node.send_cmd("LED_ON")).pack(side=tk.LEFT, padx=4)
        tk.Button(top, text="LED_OFF", width=10, command=lambda: self.node.send_cmd("LED_OFF")).pack(side=tk.LEFT, padx=4)

        self.log = ScrolledText(self, height=22)
        self.log.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)
        self.status = tk.Label(self, anchor='w')
        self.status.pack(fill=tk.X, padx=8, pady=(0,8))

        self.node = start_ros(self.append_msg)
        self.after(1000, self.update_status)

    def append_msg(self, kind, data):
        def _():
            self.log.insert(tk.END, f"[{kind}] {data}\n")
            self.log.see(tk.END)
        self.after(0, _)

    def update_status(self):
        self.status.config(text="Agent: Start am HOST (COM4) oder im Container (UDP). Topics: /esp/dummy, /gui/cmd, /gui/echo")
        self.after(1000, self.update_status)


def main():
    app = App()
    app.mainloop()


if __name__ == "__main__":
    main()
