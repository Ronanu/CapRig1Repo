# immer vom Projektroot (eine Ebene über /scripts) aus laufen
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))


param(
  [Parameter(Mandatory=$true)][string]$Port,
  [int]$Baud = 921600,
  [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'
try {
  $req = @(
    "build/bootloader/bootloader.bin",
    "build/partition_table/partition-table.bin",
    "build/esp32_microros_udp_min.bin"
  )
  $missing = $req | Where-Object { -not (Test-Path -LiteralPath $_) }
  if ($missing) {
    throw "Missing build artifacts:`n$($missing -join "`n")`nRun the build first."
  }

  python -m pip show esptool | Out-Null
  if ($LASTEXITCODE -ne 0) {
    python -m pip install -U esptool pyserial
  }

  Write-Host "Flashing to $Port at $Baud baud..." -ForegroundColor Cyan
  $args = @('esptool','--chip','esp32','--port', $Port,'--baud', $Baud,'write_flash','-z',
    '0x1000','build/bootloader/bootloader.bin',
    '0x8000','build/partition_table/partition-table.bin',
    '0x10000','build/esp32_microros_udp_min.bin')
  Write-Host ("python -m " + ($args -join ' ')) -ForegroundColor DarkGray
  $p = Start-Process -FilePath 'python' -ArgumentList @('-m') + $args -NoNewWindow -Wait -PassThru
  if ($p.ExitCode -ne 0) { throw "esptool exited with code $($p.ExitCode)." }

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