#!/usr/bin/env bash
set -eo pipefail
source /opt/ros/humble/setup.bash
source /root/ros2_ws/install/setup.bash

echo "[caprig] Starting esp_bridge node ..."
ros2 run esp_bridge ros_side
