# Assembly / C++ ratio

Scope: all src/*.cpp and src/*.h, including the separate assembly proof (even code behind preprocessor guards).
Counts physical nonblank, noncomment lines. C++ braces and preprocessor lines count as C++.
Assembly instructions inside __asm blocks and single-line __asm statements count as assembly.
The __asm block wrapper and brace-only lines count as C++ syntax, never as instructions.
A line containing an assembly instruction counts once as assembly. Use one instruction per line.
C/C++ comments, assembly semicolon comments, and tokens inside string/character literals are ignored.
This scanner supports ordinary C++ quoted literals, not raw or multiline string literals; the project uses neither.

| File | Assembly lines | C++ lines | Assembly % | C++ % |
| --- | ---: | ---: | ---: | ---: |
| asm_test.cpp | 3 | 17 | 15.00 | 85.00 |
| console.cpp | 0 | 85 | 0.00 | 100.00 |
| console.h | 0 | 18 | 0.00 | 100.00 |
| data.cpp | 0 | 15 | 0.00 | 100.00 |
| game.cpp | 0 | 0 | 0.00 | 0.00 |
| game.h | 0 | 47 | 0.00 | 100.00 |
| grid.cpp | 0 | 0 | 0.00 | 0.00 |
| main.cpp | 0 | 13 | 0.00 | 100.00 |
| match.cpp | 0 | 0 | 0.00 | 0.00 |
| score.cpp | 0 | 0 | 0.00 | 0.00 |
| shooter.cpp | 0 | 0 | 0.00 | 0.00 |
| TOTAL | 3 | 195 | 1.52 | 98.48 |

Milestone 1 is a C++ foundation with only three real assembly instructions.
This is below the final minimum of 70% and target of 72-75%. Gameplay assembly is deferred.
No pointless instructions were added to increase the percentage.
