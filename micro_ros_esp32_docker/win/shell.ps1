# shell.ps1  .\win\shell.ps1
# Open an interactive shell in the running container with ROS 2 sourced.
# Window stays open at the end.

$ErrorActionPreference = "Continue"
Write-Host "=== micro-ROS Container Shell ==="

Push-Location (Join-Path $PSScriptRoot "..\docker")
try {
    docker compose exec ros2_agent bash -lc "source /opt/ros/humble/setup.bash && echo 'ROS 2 sourced' && exec bash"
} catch {
    Write-Error "Failed to open shell. Is the container running?"
}
Pop-Location

Write-Host ""
Write-Host "Press ENTER to exit..."
Read-Host | Out-Null
