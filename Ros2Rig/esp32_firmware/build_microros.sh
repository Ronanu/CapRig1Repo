#!/usr/bin/env bash
set -Eeuo pipefail

PIO_EXE="$HOME/.platformio/penv/Scripts/platformio.exe"
LOGDIR="./_logs"; mkdir -p "$LOGDIR"
LOGFILE="$LOGDIR/build_$(date +%Y%m%d_%H%M%S).log"
pause() { echo; read -n1 -s -r -p ">>> Taste drücken zum Schließen ..."; echo; }

# in Projekt wechseln
cd "$(dirname "$0")"

echo ">>> Starte micro-ROS Build (Log: $LOGFILE)"
{
  echo "=== $(date) ==="
  echo "WD: $(pwd)"
  "$PIO_EXE" --version
  echo

  echo ">>> Deep-Clean der dev-Repos"
  ./deep_clean_microros.sh || true

  echo
  echo ">>> clean_microros (PIO)"
  "$PIO_EXE" run --target clean_microros

  echo
  echo ">>> Build"
  "$PIO_EXE" run
} 2>&1 | tee "$LOGFILE"

echo
echo ">>> ✅/❌ siehe oben. Log: $LOGFILE"
pause
