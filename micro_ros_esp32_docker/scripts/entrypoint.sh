#!/usr/bin/env bash
# Robust entrypoint: sources ROS 2 and micro-ROS (if available), then starts the UDP agent.
# Verhindert Container-Crashs durch Retry-Loop mit Backoff.

# kein -e, damit wir Fehler selbst behandeln; pipefail ja, nounset erst NACH dem Sourcen
set -o pipefail

echo "[entrypoint] Starting entrypoint script..."

# --- ROS 2 Humble sourcen (nounset TEMPORÄR aus) ---
set +u
if [ -f /opt/ros/humble/setup.bash ]; then
  # shellcheck disable=SC1091
  source /opt/ros/humble/setup.bash
  echo "[entrypoint] Sourced: /opt/ros/humble/setup.bash"
else
  echo "[entrypoint] WARNING: /opt/ros/humble/setup.bash not found"
fi

# --- micro-ROS sourcen (falls vorhanden) ---
# typische Pfade: /uros_ws oder /opt/uros_ws
if [ -f /uros_ws/install/setup.bash ]; then
  # shellcheck disable=SC1091
  source /uros_ws/install/setup.bash
  echo "[entrypoint] Sourced: /uros_ws/install/setup.bash"
elif [ -f /opt/uros_ws/install/setup.bash ]; then
  # shellcheck disable=SC1091
  source /opt/uros_ws/install/setup.bash
  echo "[entrypoint] Sourced: /opt/uros_ws/install/setup.bash"
else
  echo "[entrypoint] INFO: No micro-ROS workspace setup found; using system package if available."
fi
set -u

# --- Environment defaults ---
PORT="${AGENT_PORT:-8888}"
BIND="${AGENT_BIND:-0.0.0.0}"
VERB="${AGENT_VERBOSITY:-6}"

echo "[entrypoint] target: udp4 --port ${PORT} -i ${BIND} -v${VERB}"

# --- Startfunktion für den Agent (ohne exec, damit Retry greift) ---
start_agent() {
  if command -v micro_ros_agent >/dev/null 2>&1; then
    echo "[entrypoint] Using binary: micro_ros_agent"
    micro_ros_agent udp4 --port "${PORT}" -i "${BIND}" -v"${VERB}"
    return $?
  elif command -v ros2 >/dev/null 2>&1; then
    echo "[entrypoint] Using: ros2 run micro_ros_agent micro_ros_agent"
    ros2 run micro_ros_agent micro_ros_agent udp4 --port "${PORT}" -i "${BIND}" -v"${VERB}"
    return $?
  else
    echo "[entrypoint] ERROR: Neither 'micro_ros_agent' nor 'ros2' found in PATH"
    return 127
  fi
}

# --- Retry-Loop mit Backoff ---
BACKOFF=2
MAX_BACKOFF=60
while true; do
  start_agent
  RC=$?
  echo "[entrypoint] Agent exited with code ${RC}. Retrying in ${BACKOFF}s..."
  sleep "${BACKOFF}"
  if [ "${BACKOFF}" -lt "${MAX_BACKOFF}" ]; then
    BACKOFF=$((BACKOFF * 2))
    if [ "${BACKOFF}" -gt "${MAX_BACKOFF}" ]; then BACKOFF=${MAX_BACKOFF}; fi
  fi
done
