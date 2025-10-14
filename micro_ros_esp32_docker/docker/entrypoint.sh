#!/usr/bin/env bash
set -e

DEV="${SERIAL_DEV:-/dev/ttyACM0}"
VERB="${AGENT_VERBOSITY:-6}"

echo "[micro-ros] using device: $DEV"
echo "[micro-ros] ROS_DOMAIN_ID: ${ROS_DOMAIN_ID:-0}"

if [ ! -e "$DEV" ]; then
  echo "[micro-ros] WARN: $DEV not found yet."
fi

exec micro-ros-agent serial --dev "$DEV" -v"$VERB"
