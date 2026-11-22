
@echo off
setlocal

set "OUTPUT=Complete_Source_Code.txt"

echo Exporting source code...

> "%OUTPUT%" echo COAL BUBBLE SHOOTER - COMPLETE SOURCE CODE

for %%F in (src\*.cpp src\*.h) do (
    >> "%OUTPUT%" echo.
    >> "%OUTPUT%" echo ==================================================
    >> "%OUTPUT%" echo FILE: %%~nxF
    >> "%OUTPUT%" echo ==================================================
    type "%%F" >> "%OUTPUT%"
    >> "%OUTPUT%" echo.
)

echo.
echo Source code exported to %OUTPUT%
notepad "%OUTPUT%"

pause
