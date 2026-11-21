@echo off
setlocal
rem Owner: Khizar Sheikh. Use the VS 2026 x86 Native Tools prompt (v145).
pushd "%~dp0.."
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    echo ERROR: Open the x86 Native Tools Command Prompt for VS 2026.
    popd
    exit /b 1
)
if not exist "bin" mkdir "bin"
if not exist "obj\MatchTests" mkdir "obj\MatchTests"
rem Save a compiler-generated listing as text for frame/stack review, never as assembly input.
cl /nologo /W4 /WX /EHsc /Od /Zi /Gd /RTC1 /c /FAs /Faobj\MatchTests\match_listing.txt /Fo:obj\MatchTests\match.obj /Fd:obj\MatchTests\compile.pdb src\match.cpp
if errorlevel 1 goto failed
cl /nologo /W4 /WX /EHsc /Od /Zi /Gd /RTC1 /Fo:obj\MatchTests\ /Fd:obj\MatchTests\compile.pdb /Fe:bin\MatchTests.exe tests\match_tests.cpp src\grid.cpp src\data.cpp src\console.cpp src\score.cpp src\shooter.cpp obj\MatchTests\match.obj /link /MACHINE:X86 /PDB:bin\MatchTests.pdb
if errorlevel 1 goto failed
bin\MatchTests.exe
if errorlevel 1 goto failed
popd
exit /b 0
:failed
popd
exit /b 1
