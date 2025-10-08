# immer vom Projektroot (eine Ebene über /scripts) aus laufen
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))


param(
    [switch]$Clean = $false,
    [string]$IdfImage = 'espressif/idf:release-v5.2',
    [string]$Dns = '8.8.8.8',
    [string]$BuildVolume = 'esp32_microros_build',
    [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'
try {
    if (-not (Test-Path -LiteralPath ".\CMakeLists.txt")) {
        throw "Please run this script from your project root (where CMakeLists.txt is)."
    }

    if (Test-Path -LiteralPath ".\build") {
        if ($Clean) {
            Write-Host "Removing local .\build (to avoid Windows/Dropbox locks)..." -ForegroundColor Yellow
            Remove-Item -Recurse -Force .\build
        } else {
            Write-Host "Note: A local .\build directory exists. To avoid Windows/Dropbox lock issues," -ForegroundColor Yellow
            Write-Host "      run again with -Clean to remove it, or pause Dropbox during build." -ForegroundColor Yellow
        }
    }

    $bash = @'
set -e
python -m pip install -U pip
python -m pip install -U colcon-common-extensions catkin_pkg lark-parser empy vcstool
git -C components/micro_ros_espidf_component rev-parse --is-inside-work-tree 2>/dev/null || (cd components/micro_ros_espidf_component && git clone https://github.com/micro-ROS/micro_ros_espidf_component.git .)
idf.py set-target esp32
idf.py build
'@

    $dockerArgs = @(
        'run','--rm','-it',
        '--dns', $Dns,
        '-e','IDF_COMPONENT_MANAGER=0',
        '-v', "$PWD:/project",
        '-v', "$BuildVolume:/project/build",
        '-w','/project',
        $IdfImage,
        '/bin/bash','-lc', $bash
    )

    Write-Host "Running build in Docker..." -ForegroundColor Cyan
    Write-Host ("docker " + ($dockerArgs -join ' ')) -ForegroundColor DarkGray

    $p = Start-Process -FilePath 'docker' -ArgumentList $dockerArgs -NoNewWindow -Wait -PassThru
    if ($p.ExitCode -ne 0) {
        throw "Docker build exited with code $($p.ExitCode)."
    }

    Write-Host "`nBuild finished. Artifacts should be in the Docker volume '$BuildVolume' and visible under .\build" -ForegroundColor Green
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