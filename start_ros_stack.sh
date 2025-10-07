#!/usr/bin/env bash
set -euo pipefail
source /opt/ros/humble/setup.bash
source /root/ros2_ws/install/setup.bash

AGENT_PORT="${AGENT_PORT:-8888}"

echo "[caprig] Starting micro-ROS agent on udp4:$AGENT_PORT ..."
micro-ros-agent udp4 --port "$AGENT_PORT" &
AGENT_PID=$!

sleep 1

echo "[caprig] Starting esp_bridge node ..."
ros2 run esp_bridge ros_side &
NODE_PID=$!

trap "echo; echo '[caprig] Stopping...'; kill $NODE_PID $AGENT_PID 2>/dev/null || true; wait; exit 0" INT TERM

wait -n $AGENT_PID $NODE_PID || true
