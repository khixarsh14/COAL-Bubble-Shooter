# Testing record

## Playable integration verification

The current test commands are in the README. Verified on VS 2026 MSVC x86 using the batch scripts:

- Assembly proof and game build passed.
- GameTests passed 29 checks covering motion, corner crossings, landing, scores, drops, levels, game states, and decimal conversion.
- MatchTests passed 25 checks and GridTests passed 31 checks.
- The live console smoke test passed six checks covering menu, help, drawing, shot erasure, pause/resume, and clean exit.

The material below records earlier milestones. The current source ratio is generated in RATIO.md.

Owner: Abdullah bin Faheem; Grid and Match checks contributed for Khizar's modules.
21 November 2026. Khizar confirmed the Milestone 1 preview runs in VS 2026 and
both existing projects use v145. The requested Match milestone is now complete.
Shooter/gameplay integration has not been started.

## Match milestone: actual staged results

Raw captured build and runtime output is in [MATCH_TEST_OUTPUT.md](MATCH_TEST_OUTPUT.md).
No test assertion failed or was weakened. One initial test build failed before
execution because /Fa: was an invalid listing-path spelling; it was corrected.
The final script compiles match.cpp separately, saving a generated listing as
obj/MatchTests/match_listing.txt. No external .asm source is used as build input.

| Stage, in execution order | Match tests | Existing regressions |
| --- | --- | --- |
| 1: recursive Match_Fill only | 10 checks, 0 failures | build.bat passed; proof=12; GridTests 31/0; PreviewSmoke 4 PASS |
| 2: add Match_Resolve | 17 checks, 0 failures | build.bat passed; proof=12; GridTests 31/0; PreviewSmoke 4 PASS |
| 3: add real Match_DropFloating | 25 checks, 0 failures | build.bat passed; proof=12; GridTests 31/0; PreviewSmoke 4 PASS |
| Final VS 2026 solution build and execution | Four v145 Debug|Win32 projects built; solution-built MatchTests 25/0 | GridTests 31/0; PreviewSmoke 4 PASS; proof=12; all five executable PE headers verified x86 |

Stage 2 used a temporary no-op DropFloating body so the intermediate production
build could link while pop logic was tested before connectivity removal. The
scaffold was replaced completely in stage 3. All stage-2 tests passed again with
the real floating implementation. Existing GridTests and PreviewSmoke sources
and behavior were left unchanged.

The 22 requested test cases map to these actual checks:

| Requested test | Executed evidence |
| --- | --- |
| 1: exactly 3 | Resolve removes 3 and adds 30 to score 100 |
| 2: only 2 | Grid, floaters and score unchanged |
| 3: more than 3 | Five-cell connected component removed for 50 points |
| 4: adjacent different colors | TestFill excludes them in mode 0; mixed-color ceiling chain remains after a different group pops |
| 5: diagonals | Fill excludes diagonals; Resolve cannot match a diagonal-only group |
| 6: boundaries | All four corners plus out-of-range calls exercised; bounds checks reviewed before indexing |
| 7: visited reuse | Repeating Fill on a visited start leaves count unchanged |
| 8: full 208 cells | Immediately after Fill: popCount=208, visited sum=208. Resolve empties grid and adds exactly 2080; no expectation of 208 after connectivity search |
| 9: genuine deep path | 118-cell one-wide snake, empty separator rows, alternating end connectors; every occupied cell visited and only those cells |
| 10: ESP | MOV reads ESP around a normal C++ call on the full grid; equality passes. Listing reviewed for each of four recursive calls: four pushes then ADD ESP,16; /RTC1 checks each Fill return |
| 11: mixed-color attachment | Six-cell mixed-color ceiling chain retained |
| 12: floating removal after pop | Popping a three-cell support removes two detached bubbles while preserving a ceiling anchor |
| 13: nonzero initial score | 100 + 3*10 = 130 |
| 14: pop 3 and drop 2 | 100 + 3*10 + 2*20 = 170 |
| 15: grid becomes empty | Verified by the real Grid_IsEmpty after successful pops |
| 16: no-match must not drop | Two-match leaves disconnected bottom-row bubble and score 900 unchanged |
| 17: empty start | Grid, score, visited and popCount unchanged |
| 18: invalid coordinates | Negative, oversized, INT_MIN/MAX inputs leave grid, score and scratch unchanged |
| 19: repeated resolution | No duplicate score on an empty cell; new groups ignore stale match/connectivity marks |
| 20: GridTests regression | Original 31 checks pass after each stage |
| 21: preview regression | Normal build and original 4 real-console checks pass after each stage |
| 22: x86 execution | MSVC v145 builds, /Od /RTC1 /W4 /WX harness; all actual programs run and x86 PE headers checked |

No unresolved compilation or runtime problems were observed. The solution build
used the same approved outside-sandbox MSBuild path as Milestone 2 because its
file tracker is restricted inside the sandbox. No toolset or existing project
settings were changed. Human F5 inspection and terminal visual quality remain
manual checks, rather than claimed automated results.

Current ratios from tools/ratio.ps1:

| Scope | Assembly | C++ | Assembly % | C++ % |
| --- | ---: | ---: | ---: | ---: |
| Production src/ (final requirement scope) | 268 | 264 | 50.38 | 49.62 |
| Tests only | 2 | 416 | 0.48 | 99.52 |
| Combined src/ + tests/ | 270 | 680 | 28.42 | 71.58 |

The two test assembly lines read ESP; they are not counted as production code.
The full production source still falls below the final minimum 70% / target
72-75%. match.cpp itself is 148 assembly source lines / 19 C++ lines (88.62%).
Labels count as assembly source, not as executed instructions. No padding added.
The dual-scope ratio aggregation passed a separate fixture: production 2/4,
tests 1/3, combined 3/7 (30%). An initial direct PowerShell invocation was blocked
by execution policy; its wrapper printed an invalid success message after
nonterminating errors. That message was discarded. The rerun used the documented
process-local ExecutionPolicy Bypass, stopped on any error, and asserted every
expected row successfully. No system-wide execution policy was changed.

## Run MatchTests and inspect recursion in VS 2026

From **x86 Native Tools Command Prompt for VS 2026**, with installed v145:

```bat
cd /d "C:\Users\Khizar\COAL PROJECT\COAL-Bubble-Shooter"
tests\build_match_tests.bat
```

Expect 25 PASS lines and `Match tests: 25 checks, 0 failures`, exit code 0.
The script compiles the real Match/Grid/data/console modules with /Od, /RTC1,
/W4 and /WX. It does not change or invoke the normal preview's main function.

To rerun all current regressions:

```bat
build.bat
tests\build_match_tests.bat
tests\build_grid_tests.bat
```

To regenerate both ratio measurements from PowerShell:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools\ratio.ps1
```

Open BubbleShooter.sln in VS 2026; choose Debug | Win32 (v145), Build Solution,
then set MatchTests as startup project. Ctrl+F5 runs without debugging. For viva
practice use these steps with F5:

1. Set a breakpoint in Match_Fill near the bounds checks.
2. Open Debug > Windows > Call Stack and Debug > Windows > Registers.
3. Open Debug > Windows > Memory > Memory 1; enter ESP as the address (or use the
   register's displayed value). Watch ESP, EBP, col, row, visited, and popCount.
4. Step instruction by instruction in Disassembly. Observe the four argument
   pushes, then step into CALL Match_Fill. The Call Stack gains a child frame.
5. Observe ESP decrease as arguments, return address, and saved EBP are pushed.
   This is what downward stack growth means: new data uses lower addresses.
6. Select parent and child Call Stack entries to see separate col/row parameters.
   The reviewed /Od frame has col at EBP+8, row +12, color +16, mode +20.
7. Follow a marked cell's repeated visit: its early jump stops recursion without
   increasing popCount. Follow an unvisited neighbor: it is marked before calls.
8. Step through the compiler's return: POP EBP restores the parent frame, RET
   resumes after CALL, then ADD ESP,16 discards the four arguments. Verify ESP
   returns to the parent's pre-push value. Watch count 208 immediately after the
   full-grid Fill; do not expect that count after Resolve calls connectivity mode.

The text compiler listing verified the offsets and all six Fill call sites
(four recursive, one Resolve, one DropFloating). The conventional diagram and
worked example are in CODE_WALKTHROUGH.md. No production frame is hand-written
or naked. /Od is retained in all four solution projects.

## Historical Milestone 2 actual results

The following is the preserved grid milestone record; its old current ratio
and project-count descriptions are historical and superseded above.

| Check actually run | Result | Evidence |
| --- | --- | --- |
| Initial repository inspection | PASS | Clean tree on khizar tracking origin/khizar; no branch change or Git writes. |
| Line-ending normalization check | PASS | After adding .gitattributes, git hash-object --path for every tracked file matched git rev-parse :file. No unexpected normalized-content changes. No git add --renormalize or staging was performed. |
| Normal build.bat using v145 x86 | PASS | Selected MSVC 14.51.36231 with vcvarsall x86 -vcvars_ver=14.51. Both programs compiled, proof printed 12. |
| Isolated /W4 /WX /RTC1 grid tests | PASS | 31 checks, 0 failures; real grid.cpp, data.cpp, console.cpp linked, not mocked. |
| Init rows and colors | PASS | Levels 1,2,3,4,5,100,INT_MAX fill 5,6,7,8,8,8,8 rows; remaining bytes empty. Invalid nonpositive levels use 5 rows. Every color 1..5, visited and level unchanged. |
| Get/Set | PASS | All 208 cells and neighbors checked, including erase; negative, excessive, INT_MIN/MAX coordinates and invalid colors rejected. |
| ShiftDown / AddTopRow | PASS | Compared all 192 moved bytes against a distinct-byte snapshot; cleared top, discarded old last row, refilled top with 16 valid colors, lower rows unchanged; repeated shifts empty grid. |
| Danger / empty scans | PASS | Single occupied cell tested at each of 208 positions. Only indices 176..207 trigger danger; any nonzero byte, including final index 207, is occupied. Exact 0/1 returned. |
| Random service | PASS | 1000 real Con_Rand(5) calls in range plus safe n=0/-1 behavior. Fixed seeds reproduce initialization. |
| Normal preview smoke test | PASS | Real hidden child console contained title, borders, row-11 danger line, side labels, controls and footer. Cursor hidden, injected ESC exited with code 0, cursor visible afterward. |
| VS 2026 solution Debug|Win32 v145 | PASS | BubbleShooter, AsmProof, GridTests built successfully with installed MSBuild. Then the solution-built GridTests passed 31 checks; proof and PreviewSmoke passed again. |
| Executable architecture | PASS | BubbleShooter.exe, AsmProof.exe, GridTests.exe, PreviewSmoke.exe all have x86 PE machine 0x014C. |
| Current src ratio | MEASURED | 120 assembly source lines, 245 C++ lines; 32.88% assembly, 67.12% C++. grid.cpp alone: 117 / 44, 72.67% assembly. Labels count as assembly source; wrappers count as C++. |

The first sandboxed MSBuild attempts failed with MSB4018 / E_ACCESSDENIED in
MSBuild's FileTracker. The same normal solution command succeeded outside the
sandbox; no project workaround or toolset change was needed. No errors or
warnings were printed in the successful builds. F5 interactive debugging was
not exercised by Codex. The smoke test checks console text and cursor state,
not visual font quality, all colors, or behavior in every terminal host.

## Reproduce Milestone 2 in VS 2026

Open BubbleShooter.sln in Visual Studio 2026. Select **Debug | Win32** and verify
**Platform Toolset = v145** in project properties. Use **Build > Build Solution**.
Set **GridTests** as startup project and use **Ctrl+F5**: expect
`Grid tests: 31 checks, 0 failures`. For debugging, put a breakpoint in Grid_Get
or Grid_ShiftDown and use F5, stepping with the Disassembly/Registers windows.
Then set **BubbleShooter** as startup project, press F5, inspect the unchanged
static preview, and press ESC. No grid initialization or drawing was added to
the normal main loop. Grid functions are exercised by the isolated harness.

From **x86 Native Tools Command Prompt for VS 2026**, run:

```bat
cd /d "C:\Users\Khizar\COAL PROJECT\COAL-Bubble-Shooter"
build.bat
tests\build_grid_tests.bat
bin\BubbleShooter.exe
```

The separate test script builds and runs GridTests and PreviewSmoke. Build the
normal preview first so the smoke test uses the latest executable. PreviewSmoke
opens its child console hidden and sends ESC automatically. Expected output is
31 grid checks with zero failures and four preview PASS lines. Exit code 0 means
success; either test failure makes the script return 1. Tests live outside src
and never introduce a second main in the normal build.

Actual local developer-environment invocation used by Codex (PowerShell):

```powershell
cmd /d /s /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 -vcvars_ver=14.51 && build.bat'
cmd /d /s /c '"C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 -vcvars_ver=14.51 && tests\build_grid_tests.bat'
& 'C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe' BubbleShooter.sln /p:Configuration=Debug /p:Platform=Win32 /verbosity:minimal /nologo
powershell -NoProfile -ExecutionPolicy Bypass -File tools\ratio.ps1
```

The specified ratio scans src/*.cpp and src/*.h, including the original proof.
tests/ harnesses and tooling are excluded explicitly, not counted as game code.
The full project remains below the final 70% minimum / 72-75% target. No padding
or unrelated gameplay was added to raise it. Remaining milestones must add the
planned assembly logic before final acceptance.

## Historical Milestone 1 results

The entries below retain the original record. v143/MSB8020 and pending statuses
refer to that earlier stage; current VS 2026 results above supersede them.

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
| Ratio at least 72% | Not met | Match milestone production: 50.38%; final target pending |
| Assembly commented; required docs exist | Local pass; user pending | Three proof instructions documented |
