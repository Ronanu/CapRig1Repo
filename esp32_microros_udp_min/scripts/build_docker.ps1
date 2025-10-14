#Requires -Version 5.1
param(
    [switch]$Clean = $false,
    [string]$IdfImage = 'espressif/idf:release-v5.2',
    [string]$Dns = '8.8.8.8',
    [string]$WorkVolume = 'esp32_microros_work',
    [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'

# always run from project root (one level above /scripts)
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))
try {
    if (-not (Test-Path -LiteralPath ".\CMakeLists.txt")) {
        throw "Run this from the project root (where CMakeLists.txt is)."
    }
    if ($Clean -and (Test-Path -LiteralPath ".\dist")) { Remove-Item -Recurse -Force .\dist }

    # 1) Write the Bash build script to scripts/_docker_build.sh (host side)
    $bashPath = Join-Path $PSScriptRoot "_docker_build.sh"
    $bash = @'
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
'@
    # Write as ASCII to avoid UTF-8 BOM issues with #!/bin/bash
    Set-Content -LiteralPath $bashPath -Value $bash -Encoding Ascii -NoNewline

    # 2) Run it in Docker (host repo read-only at /host, clean work in volume /work)
    $dockerArgs = @(
        'run','--rm','-it',
        '--dns', $Dns,
        '-e','IDF_COMPONENT_MANAGER=0',
        '--mount', "type=bind,source=$($PWD.Path),target=/host,readonly",
        '--mount', "type=volume,source=$WorkVolume,target=/work",
        '-w','/work',
        $IdfImage,
        '/bin/bash','-lc', "bash /host/scripts/_docker_build.sh"

    )

    Write-Host "Running Docker build (volume: $WorkVolume)..." -ForegroundColor Cyan
    & docker @dockerArgs
    if ($LASTEXITCODE -ne 0) { throw "Docker build exited with code $LASTEXITCODE." }

    Write-Host "`nArtifacts in ./dist:" -ForegroundColor Green
    Get-ChildItem .\dist | Format-Table -AutoSize | Out-Host
}
catch {
    Write-Host "`nERROR:" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    if ($_.ScriptStackTrace) { Write-Host $_.ScriptStackTrace -ForegroundColor DarkRed }
}
finally {
    Pop-Location
    if (-not $NoPause) { Read-Host "`nPress ENTER to close" | Out-Null }
}
