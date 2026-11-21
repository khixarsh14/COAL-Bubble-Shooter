# Assembly / C++ ratio

Production scope: src/*.cpp and src/*.h, including the standalone proof and guarded source.
Combined scope: production plus tests/*.cpp and tests/*.h. Build scripts and compiler listings are excluded.
The final project assembly requirement uses the PRODUCTION ratio from CODEX_CONTEXT.md section 11. Testing code is disclosed separately.
Counts physical nonblank, noncomment lines. C++ braces and preprocessor lines count as C++.
Assembly source lines (instructions and labels) inside __asm blocks and single-line __asm statements count as assembly.
The __asm block wrapper and brace-only lines count as C++ syntax, never as instructions.
A line containing an assembly instruction counts once as assembly. Use one instruction per line.
C/C++ comments, assembly semicolon comments, and tokens inside string/character literals are ignored.
This scanner supports ordinary C++ quoted literals, not raw or multiline string literals; the project uses neither.

| File | Assembly lines | C++ lines | Assembly % | C++ % |
| --- | ---: | ---: | ---: | ---: |
| src/asm_test.cpp | 3 | 17 | 15.00 | 85.00 |
| src/console.cpp | 0 | 91 | 0.00 | 100.00 |
| src/console.h | 0 | 18 | 0.00 | 100.00 |
| src/data.cpp | 0 | 15 | 0.00 | 100.00 |
| src/game.cpp | 0 | 0 | 0.00 | 0.00 |
| src/game.h | 0 | 47 | 0.00 | 100.00 |
| src/grid.cpp | 117 | 44 | 72.67 | 27.33 |
| src/main.cpp | 0 | 13 | 0.00 | 100.00 |
| src/match.cpp | 148 | 19 | 88.62 | 11.38 |
| src/score.cpp | 0 | 0 | 0.00 | 0.00 |
| src/shooter.cpp | 0 | 0 | 0.00 | 0.00 |
| tests/grid_tests.cpp | 0 | 135 | 0.00 | 100.00 |
| tests/match_tests.cpp | 2 | 205 | 0.97 | 99.03 |
| tests/preview_smoke.cpp | 0 | 76 | 0.00 | 100.00 |
| PRODUCTION (src only) | 268 | 264 | 50.38 | 49.62 |
| TESTS ONLY | 2 | 416 | 0.48 | 99.52 |
| COMBINED (src + tests) | 270 | 680 | 28.42 | 71.58 |

Match milestone: real recursive flood-fill, popping, and floating removal are implemented; preview behavior is unchanged.
The combined percentage includes the two ESP-reading MOV lines in tests/match_tests.cpp as test assembly.
The final minimum is 70% assembly, targeting 72-75%; further gameplay assembly is deferred.
No pointless instructions were added to increase the percentage.
