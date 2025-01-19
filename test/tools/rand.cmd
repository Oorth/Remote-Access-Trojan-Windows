@echo off
setlocal enabledelayedexpansion

:: Set the length of the random string
set LENGTH=5
set CHARSET=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789
set "maindir="

:: Loop to generate the random string
for /L %%i in (1,1,%LENGTH%) do (
    set /A "RANDOM_INDEX=!random! %% 62"
    for %%c in (!RANDOM_INDEX!) do (
        set "RANDOM_CHAR=!CHARSET:~%%c,1!"
    )
    set "maindir=!maindir!!RANDOM_CHAR!"
)

:: Output the random string
echo Random String: %maindir%

:: End of script
endlocal
