#Requires -Version 5.1
param(
  [Parameter(Mandatory=$true)][string]$Port,
  [int]$Baud = 921600,
  [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'

# always run from project root (one level above /scripts)
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))
try {
  $req = @(
    "dist/bootloader.bin",
    "dist/partition-table.bin",
    "dist/esp32_microros_udp_min.bin"
  )
  $missing = $req | Where-Object { -not (Test-Path -LiteralPath $_) }
  if ($missing) {
    throw "Missing artifacts:`n$($missing -join "`n")`nRun build_docker first."
  }

  python -m pip show esptool | Out-Null
  if ($LASTEXITCODE -ne 0) {
    python -m pip install -U esptool pyserial
  }

  Write-Host "Flashing to $Port at $Baud baud..." -ForegroundColor Cyan
  $args = @('esptool','--chip','esp32','--port', $Port,'--baud', $Baud,'write_flash','-z',
    '0x1000','dist/bootloader.bin',
    '0x8000','dist/partition-table.bin',
    '0x10000','dist/esp32_microros_udp_min.bin')
  Write-Host ("python -m " + ($args -join ' ')) -ForegroundColor DarkGray
  & python -m @args
  if ($LASTEXITCODE -ne 0) { throw "esptool exited with code $LASTEXITCODE." }

  Write-Host "`nOpening serial monitor at 115200 (Ctrl+] to exit)..." -ForegroundColor Cyan
  Start-Process -FilePath 'python' -ArgumentList '-m','serial.tools.miniterm', $Port, '115200' -NoNewWindow
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