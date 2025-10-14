# down.ps1
# Stop and remove the micro-ROS agent container.
# Window stays open at the end.

$ErrorActionPreference = "Continue"
Write-Host "=== micro-ROS Docker Stop ==="

Push-Location (Join-Path $PSScriptRoot "..\docker")
try { docker compose down } catch { Write-Error "Failed to stop container. Check Docker Desktop." }
Pop-Location

Write-Host ""
Write-Host "Containers after stop:"
docker ps

Write-Host ""
Write-Host "Press ENTER to exit..."
Read-Host | Out-Null
