@echo off
setlocal
pushd "%~dp0.."
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    echo ERROR: Open the x86 Native Tools Command Prompt for VS 2026.
    popd
    exit /b 1
)
if not exist "bin" mkdir "bin"
if not exist "obj\GameTests" mkdir "obj\GameTests"
cl /nologo /W4 /WX /EHsc /Od /Zi /Gd /RTC1 /Fo:obj\GameTests\ /Fd:obj\GameTests\compile.pdb /Fe:bin\GameTests.exe tests\game_tests.cpp src\game.cpp src\shooter.cpp src\score.cpp src\console.cpp src\data.cpp src\grid.cpp src\match.cpp /link /MACHINE:X86 /PDB:bin\GameTests.pdb
if not errorlevel 1 bin\GameTests.exe
set "result=%ERRORLEVEL%"
popd
exit /b %result%
