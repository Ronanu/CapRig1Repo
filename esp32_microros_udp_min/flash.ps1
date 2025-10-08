param(
  [Parameter(Mandatory=$true)][string]$Port,
  [int]$Baud = 921600
)

$req = @(
  "build/bootloader/bootloader.bin",
  "build/partition_table/partition-table.bin",
  "build/esp32_microros_udp_min.bin"
)
$missing = $req | Where-Object { -not (Test-Path -LiteralPath $_) }
if ($missing) {
  Write-Error "Missing build artifacts: `n$($missing -join "`n")`nRun the build first."
  exit 1
}

python -m pip show esptool | Out-Null
if ($LASTEXITCODE -ne 0) {
  python -m pip install -U esptool pyserial
}

python -m esptool --chip esp32 --port $Port --baud $Baud write_flash -z `
  0x1000  build/bootloader/bootloader.bin `
  0x8000  build/partition_table/partition-table.bin `
  0x10000 build/esp32_microros_udp_min.bin

Write-Host "Opening serial monitor at 115200 (Ctrl+] to exit)..." -ForegroundColor Cyan
python -m serial.tools.miniterm $Port 115200