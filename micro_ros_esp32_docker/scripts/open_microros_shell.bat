@echo off


set "CONTAINER=microros-agent"

cmd /k docker exec -it "%CONTAINER%" bash -lc "set +u; ^
if [ -f /opt/ros/humble/setup.bash ]; then source /opt/ros/humble/setup.bash; echo '[shell] Sourced /opt/ros/humble/setup.bash'; else echo '[shell] WARNING: ROS setup not found'; fi; ^
if [ -f /uros_ws/install/setup.bash ]; then source /uros_ws/install/setup.bash; echo '[shell] Sourced /uros_ws/install/setup.bash'; ^
elif [ -f /opt/uros_ws/install/setup.bash ]; then source /opt/uros_ws/install/setup.bash; echo '[shell] Sourced /opt/uros_ws/install/setup.bash'; ^
else echo '[shell] INFO: No micro-ROS workspace found'; fi; ^
set -u; exec bash"
