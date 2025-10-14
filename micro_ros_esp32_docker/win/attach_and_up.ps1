\
# Requires: usbipd-win, Docker Desktop (WSL2 backend enabled)
# Run this in an elevated PowerShell (Run as Administrator)

$ErrorActionPreference = "Stop"

# 1) Try to find a likely USB serial device and attach it to docker-desktop
Write-Host "Listing USB devices (usbipd wsl list):"
usbipd wsl list

# You can adjust this filter for your board (CH340 / CP210x / USB Serial Device)
# We'll try a simple heuristic: pick the first "Available" device
$devices = usbipd wsl list | Select-String -Pattern "Available"
if ($devices.Count -eq 0) {
    Write-Warning "No 'Available' USB devices found via usbipd. If your ESP32 is already attached, this is fine."
} else {
    # Extract busid from the line (first token)
    $busid = ($devices[0].ToString().Split(" ", [System.StringSplitOptions]::RemoveEmptyEntries))[0]
    Write-Host "Attempting to attach BUSID $busid to 'docker-desktop'..."
    try {
        usbipd wsl attach --busid $busid --distribution docker-desktop | Out-Host
    } catch {
        Write-Warning "Attach may have failed or device already attached. Proceeding."
    }
}

# 2) Bring up the micro-ROS agent container
Push-Location (Join-Path $PSScriptRoot "..\docker")
Write-Host "Starting container (docker compose up -d)..."
docker compose up -d
Pop-Location

Write-Host "Done. The micro-ROS Agent should now be running."
