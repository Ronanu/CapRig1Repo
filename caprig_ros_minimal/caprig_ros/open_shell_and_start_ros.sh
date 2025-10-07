#!/usr/bin/env bash
set -euo pipefail
CTR_NAME="caprig_ros"
docker exec -it "$CTR_NAME" bash -lc "start_ros_stack.sh"
