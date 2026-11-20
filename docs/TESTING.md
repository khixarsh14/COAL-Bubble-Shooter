# Testing record

Owner: Abdullah bin Faheem. Milestone 1 only, 21 November 2026.
Human build/run confirmation is pending. No later milestone is authorized yet.

## Checks actually performed by Codex

| Check | Result | Evidence / limit |
| --- | --- | --- |
| MSVC x86 build.bat | PASS | Both executables compiled with no warnings/errors printed, using MSVC 14.44.35207 selected from a VS 2026 installation. This was not the VS 2022 IDE. |
| Run assembly proof | PASS | Printed `MSVC x86 inline assembly: 7 + 5 = 12`; returned exit code 0, both during build and on a separate run. |
| Binary architecture | PASS | PE machine field is 0x014C (x86) in both executables. |
| Solution project XML | PASS | Both .vcxproj files parsed as XML. Both define Debug|Win32 with v143. |
| Solution build on installed MSBuild | BLOCKED | MSB8020: VS 2022 v143 tools not registered in the installed VS 2026 MSBuild. No successful solution/IDE build is claimed. |
| Ratio tool on project | PASS | 3 assembly and 195 C++ lines, 1.52% assembly. Final target intentionally not met by this foundation. |
| Ratio tool fixture | PASS | Separate ignored obj/ratio-fixture source: block/line/semicolon comments, quoted __asm text, block and one-line __asm, and next-line opening brace. Expected and actual: 4 assembly, 6 C++ (40%). |
| Whitespace diff check | PASS | git diff --check returned no errors for tracked changes. |
| Interactive console preview | NOT TESTED | No claim that visual rendering, ESC handling, or cursor/title/color restoration was observed. |

Local build command used for the successful compiler check, from PowerShell:

```powershell
cmd /d /s /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 -vcvars_ver=14.44 && build.bat'
```

The optional solution build attempted was:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' BubbleShooter.sln /p:Configuration=Debug /p:Platform=Win32 /verbosity:minimal /nologo
```

## Khizar's Milestone 1 checklist

Open **x86 Native Tools Command Prompt for VS 2022**:

```bat
cd /d "C:\Users\Khizar\COAL PROJECT\COAL-Bubble-Shooter"
build.bat
bin\AsmProof.exe
bin\BubbleShooter.exe
```

| Check | Pass/fail | Notes |
| --- | --- | --- |
| build.bat compiles with no errors in VS 2022 x86 prompt | Pending | Paste complete errors if any |
| Proof displays 7 + 5 = 12 | Pending | Program should return zero |
| Title and complete ASCII boundaries are visible | Pending | Use a normal console window at least 80x25 |
| Empty grid area and row-11 danger line are aligned | Pending | Row zero begins at screen y=2, danger at y=13 |
| Side panel, placeholders, launcher, and controls are visible | Pending | No playable bubbles yet |
| Frame stays still without repeated clearing | Pending | No gameplay redraw loop |
| Left/Right, Space, and P do not start gameplay | Pending | Expected in Milestone 1 |
| ESC exits preview with cursor, colors, and title restored | Pending | Window dimensions and old contents are not restored |
| VS 2022 Debug|Win32 solution builds and F5 works | Pending | Requires Desktop development with C++, v143, Windows SDK |
| Console setup failure reports error and exits with code 1 | Pending | Redirected/non-console output may trigger this |

## Final acceptance checklist from section 13 (deferred)

These are retained for the final project. Do not mark them passed based on the
static foundation. No gameplay has been implemented or tested.

| Check | Pass/fail | Status |
| --- | --- | --- |
| Builds without errors using build.bat (x86) | Local pass; user pending | See toolset limits above |
| Menu: 1 starts, 2 help, 3 exits | Pending | Deferred |
| Grid shows colored bubbles and matches layout | Pending | Deferred; empty frame only |
| Left/Right aims across 9 angles, bounded | Pending | Deferred |
| Space fires; wall bounce on both sides | Pending | Deferred |
| Correct empty-cell and ceiling snapping | Pending | Deferred |
| 3+ matching connected bubbles pop; 2 do not | Pending | Deferred |
| Disconnected bubbles fall and score extra | Pending | Deferred |
| Score, level, high score display and update | Pending | Deferred |
| Next/current bubble preview handling | Pending | Deferred |
| DROP IN, row shift, new row, reset after pop | Pending | Deferred |
| Danger row ends game; final score appears | Pending | Deferred |
| Pause/resume, ESC menu, clean restart | Pending | Deferred |
| Empty grid advances level with more rows | Pending | Deferred |
| No flicker during play or long-play crash | Pending | Deferred |
| Ratio at least 72% | Not met | Milestone 1 exception; 1.52% now |
| Assembly commented; required docs exist | Local pass; user pending | Three proof instructions documented |
