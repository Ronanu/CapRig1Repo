# CapRig — Minimal micro-ROS ↔ ROS 2 example

- ROS 2 Humble in Docker (container `caprig_ros`)
- micro-ROS ESP32 (Device name `device_1`)
- UDP agent port: 8888

## Topics
- ESP → ROS: `/esp/heartbeat_us` (`std_msgs/UInt64`)
- ROS → ESP: `/esp/echo_request` (`std_msgs/String`)
- ESP → ROS: `/esp/echo_response` (`std_msgs/String`)

## Quick start (Windows + Git Bash or WSL)
```bash
bash run_container.sh
# in another terminal:
bash open_shell_and_start_ros.sh
```

Flash the ESP32 (PlatformIO project in `esp32/`), it will connect to Wi‑Fi and the agent.
You should see heartbeats and echo responses inside the container.
