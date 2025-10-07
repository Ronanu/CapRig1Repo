#!/usr/bin/env bash
set -uo pipefail

IMG_NAME="caprig_ros:humble"
CTR_NAME="caprig_ros"

docker build -t "$IMG_NAME" .

if docker ps -a --format '{{.Names}}' | grep -q "^${CTR_NAME}$"; then
  docker rm -f "$CTR_NAME" >/dev/null 2>&1 || true
fi

docker run --name "$CTR_NAME" \
  -p 8888:8888/udp \
  -it "$IMG_NAME"
