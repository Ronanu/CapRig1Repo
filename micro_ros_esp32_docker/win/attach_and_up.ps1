# attach_and_up.ps1
# Run in an elevated PowerShell (Run as Administrator).
# Attaches CP210x (BUSID 2-2) to docker-desktop and starts the micro-ROS container.
# Keeps window open.

$ErrorActionPreference = "Continue"
Write-Host "=== micro-ROS ESP32 Docker Start ==="

# --- Config (adjust if needed) ---
# Your CP210x device as seen by 'usbipd list'
$BUSID = "2-2"
# Inside the Linux container, CP210x typically appears as /dev/ttyUSB0
$SERIAL_DEV_IN_LINUX = "/dev/ttyUSB0"
$ROS_DOMAIN_ID = "0"
$AGENT_VERBOSITY = "6"
# ----------------------------------

# 0) Require Admin
$IsAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()
).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")
if (-not $IsAdmin) {
    Write-Error "Please run this script as Administrator."
    Write-Host "Press ENTER to exit..."
    Read-Host | Out-Null
    exit 1
}

# 1) Attach USB via usbipd (new syntax)
if (Get-Command usbipd -ErrorAction SilentlyContinue) {
    Write-Host ""
    Write-Host "[1/4] Checking USB devices (usbipd list)..."
    usbipd list

    Write-Host ("  -> Binding BUSID {0}..." -f $BUSID)
    try { usbipd bind --busid $BUSID } catch { Write-Warning "bind failed or already bound." }

    Write-Host ("  -> Attaching BUSID {0} to 'docker-desktop'..." -f $BUSID)
    try {
        usbipd attach --wsl --distribution docker-desktop --busid $BUSID
    } catch {
        Write-Warning "attach failed or already attached."
    }
} else {
    Write-Warning "[1/4] usbipd not found. Skipping USB attach."
}

# 2) Ensure docker/.env exists with proper values
Write-Host ""
Write-Host "[2/4] Ensuring docker/.env exists..."
$dockerDir = Join-Path $PSScriptRoot "..\docker"
$envPath = Join-Path $dockerDir ".env"
if (-not (Test-Path $dockerDir)) {
    New-Item -ItemType Directory -Path $dockerDir | Out-Null
}
if (-not (Test-Path $envPath)) {
    @"
ROS_DOMAIN_ID=$ROS_DOMAIN_ID
SERIAL_DEV=$SERIAL_DEV_IN_LINUX
AGENT_VERBOSITY=$AGENT_VERBOSITY
"@ | Out-File -FilePath $envPath -Encoding ASCII
    Write-Host ("  -> Created {0}" -f $envPath)
} else {
    Write-Host ("  -> Found existing {0}" -f $envPath)
}

# 3) Start container
Write-Host ""
Write-Host "[3/4] Starting micro-ROS agent container (docker compose up -d)..."
Push-Location $dockerDir
try { docker compose up -d } catch { Write-Error "Failed to run docker compose. Is Docker Desktop running?" }
Pop-Location

# 4) Show status
Write-Host ""
Write-Host "[4/4] Container status:"
docker ps --filter "name=ros2_agent"

Write-Host ""
Write-Host "If no errors above, the container should be running."
Write-Host "Flash the Arduino sketch on COM7 in the Arduino IDE if needed."
Write-Host "Press ENTER to exit..."
Read-Host | Out-Null
