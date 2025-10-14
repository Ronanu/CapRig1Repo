# micro_ros_esp32_docker (Windows Host + Docker + Arduino)

**Setup:** Arduino IDE on Windows (COM4), ROS 2 + micro-ROS Agent in a Docker container (Linux).  
Original tutorial for reference: <https://www.hackster.io/514301/micro-ros-on-esp32-using-arduino-ide-1360ca>

## Quickstart (Windows 10/11 + Docker Desktop, WSL2 backend)

### 0) Prereqs
- Install **Docker Desktop** (Linux containers).
- Install **usbipd-win** (Microsoft) for USB → WSL passthrough:
  - Admin PowerShell: `winget install usbipd` (or see GitHub microsoft/usbipd-win).
- Plug the ESP32 (shows as **COM4** for you).

### 1) Attach USB device to `docker-desktop` (once per boot)
Run (as admin) PowerShell: `win\attach_and_up.ps1`  
This will:
- Use `usbipd wsl list` to show your USB devices.
- Attempt to attach a USB serial device to the **docker-desktop** WSL2 distro.
- Start the micro-ROS Agent container via `docker compose`.

### 2) Flash Arduino Sketch (on Windows)
- Open Arduino IDE → Board: DOIT ESP32 DEVKIT V1 → Port: **COM4**.
- Install library **micro_ros_arduino** (ZIP).
- Open `arduino/sketches/micro_ros_publisher/micro_ros_publisher.ino` and upload.

> If flashing fails because the port is busy, stop the container: `win\down.ps1`, flash again, then `attach_and_up.ps1`.

### 3) Open a ROS 2 shell inside the running container
Run: `win\shell.ps1`  
You land in a bash shell with ROS 2 already sourced.

### 4) Test topics
Inside the container shell:
```bash
ros2 topic list
ros2 topic echo /micro_ros_arduino_node_publisher
```

## Notes
- The serial device path inside Linux (WSL/Docker) will look like `/dev/ttyACM0` or `/dev/ttyUSB0`. Adjust `.env` if needed.
- On each Windows reboot, you may need to re-run `attach_and_up.ps1` to re-attach the USB device to `docker-desktop`.

## Repo layout
```
micro_ros_esp32_docker/
├─ arduino/sketches/micro_ros_publisher/micro_ros_publisher.ino
├─ docker/Dockerfile
├─ docker/entrypoint.sh
├─ docker/compose.yml
├─ scripts/check_ports.sh
├─ scripts/udev-dialout-notes.md
├─ win/attach_and_up.ps1
├─ win/shell.ps1
├─ win/down.ps1
├─ .env
└─ README.md
```
