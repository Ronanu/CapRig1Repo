#!/usr/bin/env bash
set -Euo pipefail

log() { echo "[entrypoint] $*"; }

# --- nur diese zwei Setup-Skripte sourcen ---------------------------------
_try_source_strict() {
  local f="$1"
  if [[ -f "$f" ]]; then
    log "source $f"
    # ROS-Setup kann auf ungesetzte Vars zugreifen → kurz 'nounset' aus
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

# --- Agent-Konfiguration (Wi-Fi/UDP) --------------------------------------
# Hinweis: Der Agent lauscht auf UDP (Wi-Fi), Clients verbinden sich auf AGENT_HOST:AGENT_PORT.
PORT="${AGENT_PORT:-8888}"
BIND="${AGENT_BIND:-0.0.0.0}"      # 0.0.0.0 = alle Interfaces (inkl. WLAN)
VERB="${AGENT_VERBOSITY:-6}"
TRANSPORT="udp4"

# Optionale Info aus .env (für Logs nützlich, Agent selbst braucht 'HOST' nicht)
AGENT_HOST="${AGENT_HOST:-<unset>}"
log "Wi-Fi/UDP agent target for clients: ${AGENT_HOST}:${PORT}"
log "Bind address: ${BIND}, verbosity: ${VERB}"

_have() { command -v "$1" >/dev/null 2>&1; }

# --- Agent-Ermittlung (ohne Overrides, ohne Suche nach weiteren Pfaden) ---
# Reihenfolge:
# 1) 'ros2 run micro_ros_agent micro_ros_agent ...' (wenn Paket vorhanden)
# 2) 'MicroXRCEAgent ...' (Standalone)
CMD=()
if _have ros2 && ros2 pkg executables micro_ros_agent >/dev/null 2>&1; then
  CMD=(ros2 run micro_ros_agent micro_ros_agent "${TRANSPORT}" --port "${PORT}" -i "${BIND}" -v"${VERB}")
  log "using: ros2 run micro_ros_agent micro_ros_agent"
elif _have MicroXRCEAgent; then
  CMD=(MicroXRCEAgent "${TRANSPORT}" --port "${PORT}" -r "${BIND}")
  # Hinweis: MicroXRCEAgent hat keine -v Stufe wie micro_ros_agent; Standard-Logging reicht.
  log "using: MicroXRCEAgent"
else
  log "ERROR: No micro-ROS agent found (ros-humble-micro-ros-agent | micro-xrce-dds-agent)."
  log "Container stays open for debug (tail -f /dev/null)."
  tail -f /dev/null
  exit 1
fi

log "${CMD[*]}"

# --- Starten & Container offen halten -------------------------------------
set +e
# shellcheck disable=SC2294
bash -lc "${CMD[*]}" &
AGENT_PID=$!

trap 'log "SIGTERM -> stopping agent"; kill -TERM $AGENT_PID 2>/dev/null; wait $AGENT_PID; exit 0' TERM INT

wait $AGENT_PID
EXIT_CODE=$?
log "agent exited with code ${EXIT_CODE}"

# Immer offen bleiben (Debug / Logs)
log "keeping container open (tail -f /dev/null)"
tail -f /dev/null
