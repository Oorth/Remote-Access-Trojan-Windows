
@REM @echo off

SET currentDirectory=%cd%
echo Current directory: %currentDirectory%

SET TARGETPATH="C:\Users\%username%\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup"
cd %TARGETPATH%

make a message box
(echo MsgBox "Line 1", 262192, "Title") > popup.vbs

@REM cd %currentDirectory%

del initial.bat

