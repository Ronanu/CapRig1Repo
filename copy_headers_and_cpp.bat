@echo off
REM copy_headers_and_cpp.bat
REM Batch-Datei zum Ausführen des PowerShell-Scripts per Doppelklick

echo === CapRig Header und CPP Dateien Kopierer ===
echo Starte PowerShell Script...
echo.

REM PowerShell Script ausführen mit Execution Policy Bypass
powershell.exe -ExecutionPolicy Bypass -File "%~dp0copy_headers_and_cpp.ps1"

echo.
echo Script beendet. Fenster wird in 3 Sekunden geschlossen...
timeout /t 3 /nobreak >nul
