# AI assistance disclosure

Codex generated the Milestone 1 foundation on 21 November 2026 using the team's
CODEX_CONTEXT.md specification. Ownership comments identify human responsibility,
not a claim that the listed owner wrote the generated code by hand.

| Generated file(s) | Generated content / functions |
| --- | --- |
| src/game.h | Constants, global declarations, reserved gameplay interfaces |
| src/data.cpp | All global definitions and direction tables |
| src/console.h | Implemented and reserved service declarations |
| src/console.cpp | Con_Init, Con_Shutdown, Con_DrawText, Con_DrawFrame, Con_WaitForExit |
| src/main.cpp | Preview main |
| src/asm_test.cpp | Asm_Add and proof main |
| src/game.cpp, shooter.cpp, grid.cpp, match.cpp, score.cpp | Ownership/concept placeholder comments only |
| build.bat | x86 build and proof execution |
| BubbleShooter.sln, BubbleShooter.vcxproj, AsmProof.vcxproj | VS 2022 Win32 build configuration |
| tools/ratio.ps1, docs/RATIO.md | Counting script and its generated report |
| README.md | Appended Milestone 1 documentation; original introduction preserved |
| docs/CODE_WALKTHROUGH.md, OUT_OF_OUTLINE.md, AI_DISCLOSURE.md, TESTING.md | Initial documentation |

Khizar reported that the Milestone 1 preview runs in VS 2026 and that both
existing projects were retargeted to v145 before Milestone 2. Detailed human
line-by-line review is not claimed. Team members should review, build, test,
and understand every generated line before submission.

## Milestone 2 assistance

Codex generated the following changes on 21 November 2026. Ownership assignments
remain as specified; match.cpp, main.cpp, game.cpp, and shooter.cpp were unchanged.

| Files | Generated changes |
| --- | --- |
| .gitattributes | Requested line-ending rules; normalized-content comparison without staging |
| src/grid.cpp | Grid_Init, Grid_Get, Grid_Set, Grid_ShiftDown, Grid_AddTopRow, Grid_CheckDanger, Grid_IsEmpty |
| src/console.cpp | stdlib include and minimal implementation of the already declared Con_Rand service |
| tests/grid_tests.cpp | Check, TestInit, TestGetSet, TestShiftAndTop, TestScans, TestRandom, main |
| tests/preview_smoke.cpp | HasText and main for actual preview regression checks |
| tests/build_grid_tests.bat | Separate x86 test build/run commands |
| GridTests.vcxproj, BubbleShooter.sln | Isolated v145 Debug|Win32 grid test project and solution entry |
| tools/ratio.ps1 | Updated milestone/scope explanation; counting algorithm unchanged |
| docs/CODE_WALKTHROUGH.md, TESTING.md, RATIO.md, OUT_OF_OUTLINE.md, AI_DISCLOSURE.md | Grid behavior, registers, test evidence, ratio, and disclosure |

The existing BubbleShooter.vcxproj and AsmProof.vcxproj v145 configurations were
preserved. No human modifications made after this generation are claimed here.

## Match milestone assistance

Codex generated the Match milestone at Khizar's request using the attached
CODEX PROMPT 03 and the full CODEX_CONTEXT.md specification. All three Match
functions retain their original void cdecl signatures. No human review beyond
the reported earlier milestone testing is claimed.

| Files | Generated changes |
| --- | --- |
| src/match.cpp | Match_Fill (actual recursive inline assembly), Match_Resolve, Match_DropFloating; direct ownership of pop/floating score updates |
| src/game.h | Comments explaining scratch state, void results, coordinate order context, and no duplicate scoring; no signature, constant, or data changes |
| tests/match_tests.cpp | Check, Reset, VisitedSum, Occupied, TestFill, TestResolve, TestFloating, main; deterministic direct-grid tests and ESP checks |
| tests/build_match_tests.bat | Separate MSVC v145 x86 test build/run and compiler text listing |
| MatchTests.vcxproj, BubbleShooter.sln | Isolated Debug|Win32 v145 MatchTests project and solution entry |
| tools/ratio.ps1 | Production, test-only and combined summaries; source-line scanner unchanged |
| docs/RATIO.md | Actual production and combined report |
| docs/CODE_WALKTHROUGH.md, TESTING.md | Verified signatures, algorithm/stack explanations, integration contract and actual staged results |
| docs/MATCH_TEST_OUTPUT.md | Captured raw output from staged runs and solution-built programs |
| docs/AI_DISCLOSURE.md | This disclosure update |

grid.cpp, score.cpp, console.cpp/.h, data.cpp, main.cpp, shooter.cpp, game.cpp,
the existing test harnesses and three existing project files were not changed.
No new outside-outline runtime concept was introduced: recursion, cdecl stack
operations, integer scoring, and the ESP-reading test instructions are already
covered by the course. OUT_OF_OUTLINE.md therefore remains unchanged.

| Member | Date | Files/functions reviewed | Changes made by hand | Build/test evidence |
| --- | --- | --- | --- | --- |
| Usman Khan | Pending | Pending | Pending | Pending |
| Abdullah bin Faheem | Pending | Pending | Pending | Pending |
| Khizar Sheikh | Pending | Pending | Pending | Pending |

## Playable integration assistance

Codex implemented shooter motion and collision, the game state machine, scoring and drop/level helpers, live Win32 input and rendering, the main loop, and the gameplay and live console tests. Codex updated build inputs and documentation and executed the x86 batch verification. The original grid and match implementations were preserved. Human review of the integration remains to be recorded.
