#!/usr/bin/env bash
set -e

# ruhig sourcen (kein Trace)
unset AMENT_TRACE_SETUP_FILES || true

# 1) ROS Base
source "/opt/ros/${ROS_DISTRO:-humble}/setup.bash"

# 2) micro-ROS Agent Workspace
if [[ -f "/uros_ws/install/setup.bash" ]]; then
  source "/uros_ws/install/setup.bash"
else
  echo "[agent] WARN: /uros_ws/install/setup.bash not found"
fi

DEV="${1:-}"
VERB="${2:-6}"
ts() { date "+%Y-%m-%d %H:%M:%S"; }

if [[ -z "${DEV}" || ! -e "${DEV}" ]]; then
  echo "$(ts) [agent] invalid device: '${DEV}'"
  exit 0
fi

echo "$(ts) [agent] exec: ros2 run micro_ros_agent micro_ros_agent serial --dev ${DEV} -v${VERB}"
exec ros2 run micro_ros_agent micro_ros_agent serial --dev "${DEV}" -v"${VERB}"
