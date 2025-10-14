\
# Open an interactive ROS 2 shell inside the running container with ROS sourced
Push-Location (Join-Path $PSScriptRoot "..\docker")
docker compose exec ros2_agent bash -lc "source /opt/ros/humble/setup.bash && echo 'ROS 2 sourced.' && bash"
Pop-Location
