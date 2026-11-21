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
| src/console.cpp | 388 | 194 | 66.67 | 33.33 |
| src/console.h | 0 | 17 | 0.00 | 100.00 |
| src/data.cpp | 0 | 15 | 0.00 | 100.00 |
| src/game.cpp | 135 | 13 | 91.22 | 8.78 |
| src/game.h | 0 | 48 | 0.00 | 100.00 |
| src/grid.cpp | 117 | 44 | 72.67 | 27.33 |
| src/main.cpp | 0 | 19 | 0.00 | 100.00 |
| src/match.cpp | 148 | 19 | 88.62 | 11.38 |
| src/score.cpp | 64 | 26 | 71.11 | 28.89 |
| src/shooter.cpp | 167 | 18 | 90.27 | 9.73 |
| tests/game_tests.cpp | 0 | 161 | 0.00 | 100.00 |
| tests/grid_tests.cpp | 0 | 135 | 0.00 | 100.00 |
| tests/match_tests.cpp | 2 | 205 | 0.97 | 99.03 |
| tests/preview_smoke.cpp | 0 | 97 | 0.00 | 100.00 |
| PRODUCTION (src only) | 1022 | 430 | 70.39 | 29.61 |
| TESTS ONLY | 2 | 598 | 0.33 | 99.67 |
| COMBINED (src + tests) | 1024 | 1028 | 49.90 | 50.10 |
