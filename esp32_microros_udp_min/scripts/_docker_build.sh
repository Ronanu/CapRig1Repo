#!/usr/bin/env bash
set -euo pipefail

# 0) fresh workspace in volume
rm -rf /work/project /work/build
mkdir -p /work/project

# 1) copy sources from /host -> /work/project, excluding build/dist/.git (no tar -> no Dropbox races)
shopt -s dotglob
for p in /host/* /host/.[!.]*; do
  name="$(basename "$p")"
  case "$name" in
    build|dist|.git) continue ;;
  esac
  cp -a "$p" /work/project/ 2>/dev/null || true
done

# 2) deps for micro_ros_espidf_component colcon stage
python -m pip install -U pip
python -m pip install -U colcon-common-extensions catkin_pkg lark-parser empy vcstool

# Ensure the micro-ROS component is an actual repo (not only the placeholder)
if [ ! -d /work/project/components/micro_ros_espidf_component/.git ]; then
  mkdir -p /work/project/components/micro_ros_espidf_component
  git clone https://github.com/micro-ROS/micro_ros_espidf_component.git /work/project/components/micro_ros_espidf_component
fi

# 3) clean build dir and build with an out-of-tree path in the volume
cd /work/project
idf.py -B /work/build fullclean
idf.py -B /work/build set-target esp32
idf.py -B /work/build build

# 4) copy artifacts back to host
mkdir -p /host/dist
cp -v /work/build/bootloader/bootloader*.bin             /host/dist/bootloader.bin
cp -v /work/build/partition_table/partition-table.bin    /host/dist/partition-table.bin
cp -v /work/build/esp32_microros_udp_min.bin             /host/dist/esp32_microros_udp_min.bin