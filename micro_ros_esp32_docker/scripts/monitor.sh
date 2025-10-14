#!/usr/bin/env bash
# Minimaler, robuster Monitor: Container bleibt IMMER oben.
# Listet regelmäßig Devices und startet den Agenten, wenn eins da ist.

set -e  # bewusst KEIN -u / -o pipefail

ts() { date "+%Y-%m-%d %H:%M:%S"; }

SERIAL_GLOB="${SERIAL_GLOB:-/dev/serial/by-id/* /dev/ttyUSB* /dev/ttyACM*}"
VERB="${AGENT_VERBOSITY:-6}"
EXPL="${SERIAL_DEV:-}"

echo "$(ts) [monitor] started; ROS_DISTRO=${ROS_DISTRO:-humble}  ROS_DOMAIN_ID=${ROS_DOMAIN_ID:-unset}  VERBOSITY=${VERB}"
echo "$(ts) [monitor] patterns: ${SERIAL_GLOB}"
[[ -n "${EXPL}" ]] && echo "$(ts) [monitor] explicit device: ${EXPL}"

# harte Endlosschleife – darf niemals enden
while true; do
  echo
  echo "$(ts) [scan] listing candidates…"
  found_any=0
  for pat in ${SERIAL_GLOB}; do
    # ls darf fehlschlagen, ohne den Monitor zu beenden
    ls -l ${pat} 2>/dev/null && found_any=1 || true
  done
  [[ $found_any -eq 0 ]] && echo "$(ts) [scan] no candidates yet"

  # Läuft bereits ein Agent?
  if pgrep -fa "ros2 run micro_ros_agent micro_ros_agent serial" >/dev/null 2>&1; then
    echo "$(ts) [state] agent is running"
    sleep 2
    continue
  fi

  # Device ermitteln
  DEV=""
  if [[ -n "${EXPL}" && -e "${EXPL}" ]]; then
    DEV="${EXPL}"
  else
    for pat in ${SERIAL_GLOB}; do
      cand=$(ls -1 ${pat} 2>/dev/null | head -n1 || true)
      [[ -z "${cand}" ]] && continue
      if [[ -L "${cand}" ]]; then
        DEV=$(readlink -f "${cand}")
      else
        DEV="${cand}"
      fi
      [[ -n "${DEV}" && -e "${DEV}" ]] && break
    done
  fi

  if [[ -n "${DEV}" && -e "${DEV}" ]]; then
    echo "$(ts) [pick] using device: ${DEV}"
    # Agent im Hintergrund starten – Monitor bleibt oben
    /opt/agent/run_agent.sh "${DEV}" "${VERB}" &
  else
    echo "$(ts) [wait] no usable device; retry"
  fi

  sleep 2
done
