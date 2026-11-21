@echo off
setlocal
rem Owner: Khizar Sheikh. Isolated tests; normal build.bat behavior is unchanged.
pushd "%~dp0.."
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    echo ERROR: Open the x86 Native Tools Command Prompt for VS 2026.
    popd
    exit /b 1
)
if not exist "bin" mkdir "bin"
if not exist "obj\GridTests" mkdir "obj\GridTests"
cl /nologo /W4 /WX /EHsc /Od /Zi /Gd /RTC1 /Fo:obj\GridTests\ /Fd:obj\GridTests\compile.pdb /Fe:bin\GridTests.exe tests\grid_tests.cpp src\grid.cpp src\data.cpp src\console.cpp src\score.cpp src\shooter.cpp src\match.cpp /link /MACHINE:X86 /PDB:bin\GridTests.pdb
if errorlevel 1 goto failed
bin\GridTests.exe
if errorlevel 1 goto failed
rem Build the normal preview first using build.bat or the VS solution.
cl /nologo /W4 /WX /EHsc /Od /Zi /Gd /Fo:obj\GridTests\preview_smoke.obj /Fd:obj\GridTests\preview_smoke.pdb /Fe:bin\PreviewSmoke.exe tests\preview_smoke.cpp /link /MACHINE:X86 /PDB:bin\PreviewSmoke.pdb
if errorlevel 1 goto failed
bin\PreviewSmoke.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
