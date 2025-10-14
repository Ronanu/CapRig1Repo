\
# Stop and remove the micro-ROS agent container
Push-Location (Join-Path $PSScriptRoot "..\docker")
docker compose down
Pop-Location
