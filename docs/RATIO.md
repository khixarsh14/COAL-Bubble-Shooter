# Assembly / C++ ratio

Scope: all src/*.cpp and src/*.h, including the separate assembly proof (even code behind preprocessor guards).
Counts physical nonblank, noncomment lines. C++ braces and preprocessor lines count as C++.
Assembly source lines (instructions and labels) inside __asm blocks and single-line __asm statements count as assembly.
The __asm block wrapper and brace-only lines count as C++ syntax, never as instructions.
A line containing an assembly instruction counts once as assembly. Use one instruction per line.
C/C++ comments, assembly semicolon comments, and tokens inside string/character literals are ignored.
This scanner supports ordinary C++ quoted literals, not raw or multiline string literals; the project uses neither.

| File | Assembly lines | C++ lines | Assembly % | C++ % |
| --- | ---: | ---: | ---: | ---: |
| asm_test.cpp | 3 | 17 | 15.00 | 85.00 |
| console.cpp | 0 | 91 | 0.00 | 100.00 |
| console.h | 0 | 18 | 0.00 | 100.00 |
| data.cpp | 0 | 15 | 0.00 | 100.00 |
| game.cpp | 0 | 0 | 0.00 | 0.00 |
| game.h | 0 | 47 | 0.00 | 100.00 |
| grid.cpp | 117 | 44 | 72.67 | 27.33 |
| main.cpp | 0 | 13 | 0.00 | 100.00 |
| match.cpp | 0 | 0 | 0.00 | 0.00 |
| score.cpp | 0 | 0 | 0.00 | 0.00 |
| shooter.cpp | 0 | 0 | 0.00 | 0.00 |
| TOTAL | 120 | 245 | 32.88 | 67.12 |

Milestone 2 adds real inline-assembly grid operations; the static preview remains unchanged.
The required measurement scans src/ only. Isolated tests/ harnesses and build tools are excluded.
The standalone proof and shared headers remain included, as in Milestone 1.
The final minimum is 70% assembly, targeting 72-75%; further gameplay assembly is deferred.
No pointless instructions were added to increase the percentage.
