#!/usr/bin/env bash
set -euo pipefail

DEV="${SERIAL_DEV:-/dev/ttyUSB0}"
VERB="${AGENT_VERBOSITY:-6}"
SLEEP_ON_MISSING="${SLEEP_ON_MISSING:-2}"   # Sek. zwischen Checks

terminate() {
  trap - TERM INT
  if [[ -n "${AGENT_PID:-}" ]]; then
    kill "${AGENT_PID}" 2>/dev/null || true
  fi
  exit 0
}
trap terminate TERM INT

echo "[agent] watchdog up. device=${DEV} verb=${VERB}"
echo "[agent] container will stay alive even if device is absent."

while true; do
  if [[ ! -e "${DEV}" ]]; then
    echo "[agent] waiting for ${DEV} ..."
    sleep "${SLEEP_ON_MISSING}"
    continue
  fi

  echo "[agent] device present: $(ls -l "${DEV}")"
  echo "[agent] starting micro-ros-agent ..."
  micro-ros-agent serial --dev "${DEV}" -v"${VERB}" &
  AGENT_PID=$!

  # warte, bis der Agent endet (z. B. bei Kabel ab)
  wait "${AGENT_PID}" || true
  RC=$?
  echo "[agent] micro-ros-agent exited (rc=${RC}). restarting after short sleep ..."
  sleep 1
done
