# copy_headers_and_cpp.ps1
# PowerShell Script zum Kopieren aller .h und .cpp Dateien 
# aus Ordnern, die nicht mit "00" beginnen, in den 00CapRig Ordner

# Arbeitsverzeichnis auf Script-Verzeichnis setzen
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $scriptDir

Write-Host "=== CapRig Header und CPP Dateien Kopierer ===" -ForegroundColor Green
Write-Host "Arbeitsverzeichnis: $scriptDir" -ForegroundColor Yellow

# Zielordner definieren
$targetDir = "00CapRig"

# Prüfen ob Zielordner existiert
if (-not (Test-Path $targetDir)) {
    Write-Host "Erstelle Zielordner: $targetDir" -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $targetDir -Force
}

# Alle Ordner im aktuellen Verzeichnis durchsuchen
$directories = Get-ChildItem -Directory | Where-Object { $_.Name -notlike "00*" -and $_.Name -ne ".git" }

Write-Host "`nGefundene Ordner (ohne 00* und .git):" -ForegroundColor Cyan
foreach ($dir in $directories) {
    Write-Host "  - $($dir.Name)" -ForegroundColor White
}

$copiedFiles = 0

# Durch alle gefundenen Ordner iterieren
foreach ($directory in $directories) {
    Write-Host "`nDurchsuche Ordner: $($directory.Name)" -ForegroundColor Yellow
    
    # Header-Dateien (.h) suchen und kopieren
    $headerFiles = Get-ChildItem -Path $directory.FullName -Filter "*.h" -Recurse
    foreach ($file in $headerFiles) {
        $destinationPath = Join-Path $targetDir $file.Name
        
        if (Test-Path $destinationPath) {
            Write-Host "  Überschreibe: $($file.Name)" -ForegroundColor Orange
        } else {
            Write-Host "  Kopiere: $($file.Name)" -ForegroundColor Green
        }
        
        Copy-Item -Path $file.FullName -Destination $destinationPath -Force
        $copiedFiles++
    }
    
    # CPP-Dateien (.cpp) suchen und kopieren
    $cppFiles = Get-ChildItem -Path $directory.FullName -Filter "*.cpp" -Recurse
    foreach ($file in $cppFiles) {
        $destinationPath = Join-Path $targetDir $file.Name
        
        if (Test-Path $destinationPath) {
            Write-Host "  Überschreibe: $($file.Name)" -ForegroundColor Orange
        } else {
            Write-Host "  Kopiere: $($file.Name)" -ForegroundColor Green
        }
        
        Copy-Item -Path $file.FullName -Destination $destinationPath -Force
        $copiedFiles++
    }
}

Write-Host "`n=== Zusammenfassung ===" -ForegroundColor Green
Write-Host "Insgesamt kopierte Dateien: $copiedFiles" -ForegroundColor White

# Liste der kopierten Dateien im Zielordner anzeigen
Write-Host "`nDateien im $targetDir Ordner:" -ForegroundColor Cyan
$targetFiles = Get-ChildItem -Path $targetDir -Filter "*.h", "*.cpp"
foreach ($file in $targetFiles) {
    $extension = $file.Extension.ToUpper()
    $color = if ($extension -eq ".H") { "Blue" } else { "Magenta" }
    Write-Host "  $extension - $($file.Name)" -ForegroundColor $color
}

Write-Host "`nScript abgeschlossen!" -ForegroundColor Green
Write-Host "Drücken Sie eine beliebige Taste zum Beenden..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
