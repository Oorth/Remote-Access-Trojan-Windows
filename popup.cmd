@rem @echo off
@rem temp file located at the server which makes a popup and then deletes itself,,,, just to check if downloading and executing is working

SET path=%cd%
echo MsgBox "This is a popup message.", 64, "My Popup Title" > popup.vbs
start popup.vbs
cd %path%
del popup.vbs
del popup.cmd