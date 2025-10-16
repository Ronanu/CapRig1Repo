#!/usr/bin/env bash
set -Euo pipefail

# mini logger
log() { echo "[entrypoint] $*"; }

# nur diese zwei Setup-Skripte sourcen (nounset-sicher)
_try_source_strict() {
  local f="$1"
  if [[ -f "$f" ]]; then
    log "source $f"
    AMENT_TRACE_SETUP_FILES="${AMENT_TRACE_SETUP_FILES:-}"
    set +u
    # shellcheck disable=SC1090
    source "$f"
    set -u
  else
    log "ERROR: $f not found"
  fi
}

_try_source_strict "/opt/ros/humble/setup.bash"
_try_source_strict "/uros_ws/install/setup.bash"

# Wi-Fi/UDP Agent-Parameter (nur diese drei ENV sind erlaubt)
PORT="${AGENT_PORT:-8888}"
BIND="${AGENT_BIND:-0.0.0.0}"
VERB="${AGENT_VERBOSITY:-6}"

log "ros2 run micro_ros_agent micro_ros_agent udp4 --port ${PORT} -i ${BIND} -v${VERB}"

# Agent im Vordergrund starten; danach Container offen halten
set +e
ros2 run micro_ros_agent micro_ros_agent udp4 --port "${PORT}" -i "${BIND}" -v"${VERB}"
EXIT_CODE=$?
log "agent exited with code ${EXIT_CODE}"
log "keeping container open (tail -f /dev/null)"
tail -f /dev/null
