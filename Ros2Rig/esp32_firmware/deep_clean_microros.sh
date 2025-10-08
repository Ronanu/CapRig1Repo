#!/usr/bin/env bash
set -Eeuo pipefail

ROOT=".pio/libdeps/esp32dev/micro_ros_platformio/build/dev/src"
echo ">>> Deep-Clean: $ROOT"

if [[ -d "$ROOT" ]]; then
  # Dropbox-Sperren vermeiden: zuerst versuchen wir ein git-clean/reset,
  # wenn ein Repo existiert; wenn nicht, löschen wir hart.
  while IFS= read -r -d '' d; do
    echo " -> prüfe: $d"
    if [[ -d "$d/.git" ]]; then
      ( cd "$d" \
        && git reset --hard HEAD || true \
        && git clean -fdx || true )
    fi
  done < <(find "$ROOT" -mindepth 1 -maxdepth 1 -type d -print0)

  echo " -> entferne komplette dev/src"
  rm -rf "$ROOT"
else
  echo " -> dev/src nicht vorhanden – nichts zu tun."
fi

echo ">>> Deep-Clean fertig."
