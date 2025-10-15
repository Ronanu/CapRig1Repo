#!/usr/bin/env bash
set -e

# Source ROS 2 Humble (fixed path in image)
source /opt/ros/humble/setup.bash
source "/uros_ws/install/setup.bash"


# Read env or use defaults
PORT="${AGENT_PORT:-8888}"
BIND="${AGENT_BIND:-0.0.0.0}"
VERB="${AGENT_VERBOSITY:-6}"

echo "[entrypoint] micro_ros_agent udp4 --port ${PORT} -i ${BIND} -v${VERB}"
exec ros2 run micro_ros_agent micro_ros_agent udp4 --port "${PORT}" -i "${BIND}" -v"${VERB}"
