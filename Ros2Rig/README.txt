Ros2Rig – Schnellstart (Windows + ESP32 auf COM4)

Fenster 1 – ESP32 (PlatformIO)
1) Öffne Ros2Rig/esp32_firmware in VS Code (PlatformIO).
2) Upload & Monitor.

Fenster 2 – GUI (Dev-Container)
1) Öffne Ros2Rig/python_gui in VS Code.
2) Reopen in Container.
3) (Windows) Starte Host-Agent: Terminal -> Task "Start micro-ROS agent (HOST, Windows/COM4)" oder doppelklick auf start_agent_windows_host.bat
   Alternativ: nutze UDP-Task und konfiguriere den ESP auf UDP-Transport.
4) Starte die GUI: Run -> "Run GUI (container)".

Hinweis Windows:
Der micro-ROS-Agent kommt am zuverlässigsten als Host-Prozess an COM4.
ROS 2 und die Tkinter-GUI laufen vollständig im Container.
