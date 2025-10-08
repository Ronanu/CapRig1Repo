#!/usr/bin/env bash
# micro-ROS Upload & Monitor — bleibt offen & loggt

set -Eeuo pipefail

PIO_EXE="$HOME/.platformio/penv/Scripts/platformio.exe"
LOGDIR="./_logs"; mkdir -p "$LOGDIR"
LOGFILE="$LOGDIR/upload_$(date +%Y%m%d_%H%M%S).log"

pause() { echo; read -n1 -s -r -p ">>> Taste drücken zum Schließen ..."; echo; }

on_exit() {
  status=$?
  echo
  if [[ $status -ne 0 ]]; then
    echo ">>> ❌ Upload/Monitor fehlgeschlagen. Log: $LOGFILE"
  else
    echo ">>> ✅ Upload/Monitor beendet. Log: $LOGFILE"
  fi
  pause
}
trap on_exit EXIT

cd "$(dirname "$0")"

{
  echo "=== $(date) ==="
  echo "WD: $(pwd)"
  "$PIO_EXE" --version
  echo

  echo ">>> Compile"
  "$PIO_EXE" run

  echo
  echo ">>> Upload"
  "$PIO_EXE" run -t upload
} 2>&1 | tee "$LOGFILE"

echo
echo ">>> Serieller Monitor (STRG+C zum Beenden)"
"$PIO_EXE" device monitor || true
