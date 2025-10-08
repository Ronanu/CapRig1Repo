# esp32_microros_udp_min

Minimal ESP-IDF (v5.2+) + micro-ROS (UDP) project for ESP32 with Wi‑Fi reconnect,
heartbeat publisher, and echo subscriber. Agent IP is set to `192.168.1.2:8888`.

## Setup

```bash
# 0) ESP-IDF environment active

# 1) Clone micro-ROS component
cd components/micro_ros_espidf_component
git clone https://github.com/micro-ROS/micro_ros_espidf_component.git .
cd ../../

# 2) Configure and build
idf.py set-target esp32
idf.py menuconfig   # App configuration -> Wi‑Fi SSID/PW (defaults are prefilled)
idf.py build flash monitor
```

## Agent (Docker)
```bash
docker run -it --rm --net=host microros/micro-ros-agent:latest udp4 --port 8888 -v6
```

## Test
```bash
ros2 topic echo /esp/heartbeat
ros2 topic pub /esp/echo std_msgs/String "{data: 'hello'}"
```
