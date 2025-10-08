#Requires -Version 5.1
param(
    [switch]$Clean = $false,
    [string]$IdfImage = 'espressif/idf:release-v5.2',
    [string]$Dns = '8.8.8.8',
    [string]$BuildVolume = 'esp32_microros_build',
    [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'

# always run from project root (one level above /scripts)
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))
try {
    if (-not (Test-Path -LiteralPath ".\CMakeLists.txt")) {
        throw "Run this from the project root (where CMakeLists.txt is)."
    }

    if ($Clean) {
        if (Test-Path -LiteralPath ".\dist") { Remove-Item -Recurse -Force .\dist }
    }

    # Compose a bash script that builds into a Docker volume (/build_vol) and copies artifacts back to host ./dist
    $bash = @'
set -euo pipefail
python -m pip install -U pip
python -m pip install -U colcon-common-extensions catkin_pkg lark-parser empy vcstool
git -C components/micro_ros_espidf_component rev-parse --is-inside-work-tree 2>/dev/null || (cd components/micro_ros_espidf_component && git clone https://github.com/micro-ROS/micro_ros_espidf_component.git .)
idf.py set-target esp32
idf.py -B /build_vol reconfigure build
mkdir -p /project/dist
cp -v /build_vol/bootloader/bootloader*.bin /project/dist/bootloader.bin
cp -v /build_vol/partition_table/partition-table.bin /project/dist/partition-table.bin
cp -v /build_vol/esp32_microros_udp_min.bin /project/dist/esp32_microros_udp_min.bin
'@

    # Docker args (note the $() to avoid ":” parsing issues, and use $PWD.Path for a clean string path)
    $dockerArgs = @(
        'run','--rm','-it',
        '--dns', $Dns,
        '-e','IDF_COMPONENT_MANAGER=0',
        '-v', "$($PWD.Path):/project",
        '-v', "$BuildVolume:/build_vol",
        '-w','/project',
        $IdfImage,
        '/bin/bash','-lc', $bash
    )

    Write-Host "Running Docker build..." -ForegroundColor Cyan
    & docker @dockerArgs
    if ($LASTEXITCODE -ne 0) { throw "Docker build exited with code $LASTEXITCODE." }

    Write-Host "`nArtifacts copied to ./dist :" -ForegroundColor Green
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