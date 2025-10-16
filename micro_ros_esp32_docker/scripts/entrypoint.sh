#!/usr/bin/env bash
set -Euo pipefail

# --- helpers ---------------------------------------------------------------
log() { echo "[entrypoint] $*"; }

_try_source() {
  local f="$1"
  if [[ -f "$f" ]]; then
    log "source $f"
    # Some ROS setup scripts reference env vars that may be unset when 'set -u' is active.
    # Workaround: temporarily disable nounset and predefine AMENT_TRACE_SETUP_FILES.
    # shellcheck disable=SC2154
    AMENT_TRACE_SETUP_FILES="${AMENT_TRACE_SETUP_FILES:-}"
    set +u
    # shellcheck disable=SC1090
    source "$f"
    set -u
  else
    log "skip: $f not found"
  fi
}

_have() { command -v "$1" >/dev/null 2>&1; }

# --- sourcing --------------------------------------------------------------
_try_source "/opt/ros/${ROS_DISTRO:-humble}/setup.bash" || true

_try_source "${UROS_WS%/}/install/setup.bash"


# --- config ---------------------------------------------------------------
PORT="${AGENT_PORT:-8888}"
BIND="${AGENT_BIND:-0.0.0.0}"
VERB="${AGENT_VERBOSITY:-6}"
TRANSPORT="${AGENT_TRANSPORT:-udp4}"   # udp4|tcp4|serial
KEEP="${KEEP_OPEN:-true}"
OVERRIDE_CMD="${AGENT_CMD:-}"          # optional manual command

# --- diagnostics -----------------------------------------------------------
log "ROS_DISTRO=${ROS_DISTRO:-<unset>}  PATH=$PATH"
log "RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION:-<unset>}"
log "ROS_PACKAGE_PATH=${ROS_PACKAGE_PATH:-<unset>}"
if _have ros2; then
  ros2 --version || true
  ros2 pkg list | grep -E 'micro|rmw|dds' || true
else
  log "ros2 CLI not found on PATH"
fi

# --- agent selection -------------------------------------------------------
CMD=()
if [[ -n "$OVERRIDE_CMD" ]]; then
  # split override into array safely
  # shellcheck disable=SC2206
  CMD=($OVERRIDE_CMD)
  log "Using override AGENT_CMD: ${CMD[*]}"
else
  # prefer ROS 2 package executable
  if _have ros2 && ros2 pkg executables micro_ros_agent >/dev/null 2>&1; then
    CMD=(ros2 run micro_ros_agent micro_ros_agent "${TRANSPORT}" --port "${PORT}" -i "${BIND}" -v"${VERB}")
    log "resolved agent: ros2 run micro_ros_agent micro_ros_agent"
  # fallback: standalone Micro XRCE-DDS Agent
  elif _have MicroXRCEAgent; then
    CMD=(MicroXRCEAgent "${TRANSPORT}" --port "${PORT}")
    log "resolved agent: MicroXRCEAgent (standalone)"
  # fallback: direct micro_ros_agent on PATH
  elif _have micro_ros_agent; then
    CMD=(micro_ros_agent "${TRANSPORT}" --port "${PORT}" -i "${BIND}" -v"${VERB}")
    log "resolved agent: micro_ros_agent (direct)"
  else
    log "ERROR: No micro-ROS agent found."
    log "Try one of the following inside the image:"
    log "  - apt-get update && apt-get install -y ros-${ROS_DISTRO:-humble}-micro-ros-agent"
    log "  - or install standalone agent: apt-get install -y micro-xrce-dds-agent  (package name may vary)"
    log "  - or build from source: https://github.com/micro-ROS/micro-ROS-Agent"
    # keep container open even on error
    if [[ "$KEEP" == "true" ]]; then
      log "keeping container open (tail -f /dev/null)"
      tail -f /dev/null
    fi
    exit 1
  fi
fi

log "${CMD[*]}"

# --- run & keep container open --------------------------------------------
set +e  # don't terminate the entrypoint if the agent exits/fails
# shellcheck disable=SC2294
bash -lc "${CMD[*]}" &
AGENT_PID=$!

# forward signals to child so docker stop works
trap 'log "SIGTERM received -> stopping agent"; kill -TERM $AGENT_PID 2>/dev/null; wait $AGENT_PID; exit 0' TERM INT

wait $AGENT_PID
EXIT_CODE=$?
log "agent exited with code ${EXIT_CODE}"

# Keep the container open in any case (can be disabled with KEEP_OPEN=false)
if [[ "$KEEP" == "true" ]]; then
  log "keeping container open (tail -f /dev/null)"
  tail -f /dev/null
else
  exit "${EXIT_CODE}"
fi
