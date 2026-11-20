@echo off
setlocal
rem Owner: Usman Khan (integration). Run in the VS 2022 x86 Native Tools prompt.
pushd "%~dp0"
if /I not "%VSCMD_ARG_TGT_ARCH%"=="x86" (
    echo ERROR: Open the x86 Native Tools Command Prompt for VS 2022 first.
    popd
    exit /b 1
)
where cl >nul 2>nul
if errorlevel 1 goto failed
if not exist "bin" mkdir "bin"
if not exist "obj" mkdir "obj"
rem Prove inline assembly works before compiling the console foundation.
cl /nologo /W3 /EHsc /Od /Zi /Gd /DBUBBLE_ASM_TEST /Fo:obj\asm_test.obj /Fd:obj\asm_test.pdb /Fe:bin\AsmProof.exe src\asm_test.cpp /link /MACHINE:X86 /PDB:bin\AsmProof.pdb
if errorlevel 1 goto failed
bin\AsmProof.exe
if errorlevel 1 goto failed
cl /nologo /W3 /EHsc /Od /Zi /Gd /Fo:obj\ /Fd:obj\BubbleShooter.pdb /Fe:bin\BubbleShooter.exe src\*.cpp /link /MACHINE:X86 /PDB:bin\BubbleShooter.pdb
if errorlevel 1 goto failed
echo Built bin\AsmProof.exe and bin\BubbleShooter.exe for x86.
popd
exit /b 0
:failed
echo Build failed. Read the compiler or test output above.
popd
exit /b 1
