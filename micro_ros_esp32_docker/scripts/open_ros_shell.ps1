# Öffnet ein interaktives, frisch gesourcetes ROS 2/micro-ROS Bash-Terminal im laufenden Container.
# Autor: Daniel 😊

# Containername aus compose.yml
$ContainerName = "microros-agent"

# Prüfen, ob der Container läuft
$containerId = docker ps -q -f "name=$ContainerName"

if (-not $containerId) {
    Write-Host "[$ContainerName] Container not running. Starting with docker compose..."
    Push-Location (Join-Path $PSScriptRoot "docker")
    docker compose up -d $ContainerName
    Pop-Location

    # Warten, bis Container läuft
    Write-Host "Waiting for container to start..."
    Start-Sleep -Seconds 3
    $containerId = docker ps -q -f "name=$ContainerName"
    if (-not $containerId) {
        Write-Error "Container could not be started."
        exit 1
    }
}

Write-Host "[$ContainerName] Opening interactive Bash shell with sourced ROS environment..."
Write-Host ""

# Öffnet ein interaktives Bash-Fenster im Container mit ROS-Setup
docker exec -it $ContainerName bash -c "
    if [ -f /opt/ros/humble/setup.bash ]; then
        source /opt/ros/humble/setup.bash
        echo '[shell] Sourced /opt/ros/humble/setup.bash'
    else
        echo '[shell] WARNING: ROS setup not found'
    fi

    if [ -f /uros_ws/install/setup.bash ]; then
        source /uros_ws/install/setup.bash
        echo '[shell] Sourced /uros_ws/install/setup.bash'
    elif [ -f /opt/uros_ws/install/setup.bash ]; then
        source /opt/uros_ws/install/setup.bash
        echo '[shell] Sourced /opt/uros_ws/install/setup.bash'
    else
        echo '[shell] INFO: No micro-ROS workspace found'
    fi

    exec bash
"
