# COAL-Bubble-Shooter
Console-based Bubble Shooter game using x86 inline assembly and C++ for FAST-NUCES COAL Lab, Fall 2026.

## Milestone 1 only

`CODEX_CONTEXT.md` is the authoritative specification. This foundation displays a
static console frame and provides a separate assembly addition proof. There is no
gameplay yet. Wait for Khizar's build and test results before Milestone 2.

## Build and run (VS 2022)

Install Visual Studio 2022 with **Desktop development with C++**, the **MSVC v143
x86/x64 build tools**, and a Windows SDK. Open **x86 Native Tools Command Prompt
for VS 2022**, then run:

```bat
cd /d "C:\Users\Khizar\COAL PROJECT\COAL-Bubble-Shooter"
build.bat
bin\AsmProof.exe
bin\BubbleShooter.exe
```

The script checks the x86 developer environment, compiles and runs the proof
first, then compiles the frame preview. A failed compiler or proof returns a
nonzero exit code. It uses MSVC `cl`, `/Gd` (cdecl), and `/MACHINE:X86`.
The source header also rejects non-MSVC and non-x86 targets.
Output files go into ignored `bin/` and `obj/` directories.

The proof prints `MSVC x86 inline assembly: 7 + 5 = 12` and returns zero.
The preview shows the title, ASCII border, empty 16-by-13 grid area, danger line
at row 11, static launcher, side-panel placeholders, and controls. Press **ESC**
to exit the preview. The side-panel controls describe the planned game; aiming,
shooting, pause, and menus are not active. In the final game ESC will mean menu.

For Visual Studio debugging, open `BubbleShooter.sln`, select **Debug | Win32**,
set `BubbleShooter` as the startup project, and press F5. Set `AsmProof` as the
startup project to debug `Asm_Add` separately. Both projects use VS 2022's
`v143` toolset, with no x64 configuration. VS Code is the source editor.

Command-line solution build from the same VS 2022 prompt:

```bat
msbuild BubbleShooter.sln /p:Configuration=Debug /p:Platform=Win32
```

## Structure and ownership

| Files | Responsibility / owner |
| --- | --- |
| `src/game.h`, `src/data.cpp` | Shared interface and data; Usman coordinates all members |
| `src/main.cpp`, `src/game.cpp`, `src/shooter.cpp` | Usman Khan: entry, future state machine and shooting |
| `src/grid.cpp`, `src/match.cpp` | Khizar Sheikh: future grid and matching logic |
| `src/console.h`, `src/console.cpp`, `src/score.cpp` | Abdullah bin Faheem: console and future score logic |
| `src/asm_test.cpp` | Usman: isolated build proof, enabled only by `BUBBLE_ASM_TEST` |
| `build.bat`, solution and projects | Build/integration support |
| `tools/ratio.ps1` | Source line ratio report generator |
| `docs/` | Walkthrough, outside-outline explanations, disclosure, tests, ratio |

The five future gameplay `.cpp` files are comment-only placeholders. Reserved
function declarations have no definitions yet and are never called. Console
service declarations are in `console.h`, which includes the shared `game.h`.
All initial game data is defined once in `data.cpp`; arrays start empty.

## Inline assembly and course mapping

`Asm_Add` uses an `__asm` block containing `MOV`, `ADD`, and `MOV`. MSVC gives
the named parameters and local variable memory locations. EAX temporarily holds
the sum; a normal C++ return returns the saved result. No manual stack changes
or missing-return warning suppression are needed.

For later assembly procedures, a C++ function declaration plays the role of
`PROTO`, a function definition with its compiler-provided stack frame plays the
role of `PROC`, and a normal call or inline `push`/`call` sequence plays the role
of `INVOKE`. Passing a pointer supplies an address like `ADDR`. Those MASM
directives themselves do not belong inside MSVC inline assembly.

Irvine32 is not linked: it is designed for stand-alone MASM projects and is
awkward to mix with inline assembly. Win32 console services replace `Gotoxy`,
`SetTextColor`, `ReadKey`, and `Delay` (the last two services for gameplay remain
deferred). No external libraries, external `.asm` files, or floating point are used.

## Assembly ratio and validation

From PowerShell in this directory:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\ratio.ps1
```

The tool prints per-file and total assembly/C++ counts and saves `docs/RATIO.md`.
The final project requires at least 70% assembly; target 72-75%. Milestone 1 is
allowed a lower ratio: only three real instructions exist so far. Do not pad
assembly counts. The report includes the standalone proof and all shared headers;
it counts source text, including guarded code, rather than compiled machine code.

See `docs/TESTING.md` for actual validation and pending checks,
`docs/CODE_WALKTHROUGH.md` for every current function,
`docs/OUT_OF_OUTLINE.md` for Win32, blocking input, and fixed-point data,
and `docs/AI_DISCLOSURE.md` for generated work and the human review log.
