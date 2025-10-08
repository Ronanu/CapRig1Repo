# immer vom Projektroot (eine Ebene über /scripts) aus laufen
Push-Location (Resolve-Path (Join-Path $PSScriptRoot '..'))

param(
  [int]$Port = 8888,
  [string]$Tag = 'jazzy',
  [switch]$NoPause = $false
)
$ErrorActionPreference = 'Stop'
try {
  $args = @('run','-it','--rm','-p',"$Port:$Port/udp",'--name','microros-agent',"microros/micro-ros-agent:$Tag",'udp4','--port',"$Port",'-v6')
  Write-Host ("docker " + ($args -join ' ')) -ForegroundColor DarkGray
  $p = Start-Process -FilePath 'docker' -ArgumentList $args -Wait -PassThru
  if ($p.ExitCode -ne 0) { throw "docker agent exited with code $($p.ExitCode)." }
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