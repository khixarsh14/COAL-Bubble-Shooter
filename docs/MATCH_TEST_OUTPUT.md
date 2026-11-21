# Match milestone: raw console output

Captured from actual MSVC v145 x86 runs. Commands and interpretation are in TESTING.md.

The initial listing-path build failed before tests ran. The corrected script now writes a compiler listing to obj/MatchTests/match_listing.txt; no external assembly source is built.

```text
C:\Users\Khizar\COAL PROJECT\COAL-Bubble-Shooter\src\console.cpp : fatal error C1083: Cannot open compiler generated file: ':obj\MatchTests\console.asm': Invalid argument
```

## Stage 1: fill + existing regressions

```text
**********************************************************************
** Visual Studio 2026 Developer Command Prompt v18.10.2
** Copyright (c) 2026 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x86'
match_tests.cpp
match.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
PASS: Fill: exact color and four neighbours; different color and diagonal excluded
PASS: Fill: grid and score unchanged
PASS: Fill: already visited start is not counted again
PASS: Fill: mode 1 connects different colors and ignores color argument
PASS: Fill: all four corners terminate safely
PASS: Fill: invalid and empty starts preserve arrays and count
PASS: Fill: full-width mismatched color and invalid mode rejected
PASS: Fill: all 208 cells visited exactly once
PASS: Fill: top-level ESP unchanged after full-grid recursion
PASS: Fill: 118-cell one-wide serpentine path, no shortcuts or stack corruption
Match tests: 10 checks, 0 failures
asm_test.cpp
MSVC x86 inline assembly: 7 + 5 = 12
asm_test.cpp
console.cpp
data.cpp
game.cpp
grid.cpp
main.cpp
match.cpp
score.cpp
shooter.cpp
Generating Code...
LINK : bin\BubbleShooter.exe not found or not built by the last incremental link; performing full link
Built bin\AsmProof.exe and bin\BubbleShooter.exe for x86.
grid_tests.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
LINK : bin\GridTests.exe not found or not built by the last incremental link; performing full link
PASS: Init level 1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2: 6 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 3: 7 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 4: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 5: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 100: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2147483647: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 0: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -2147483648: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Get/Set every cell, including EMPTY and all five colors; no neighbor writes
PASS: Invalid coordinates return EMPTY and leave grid unchanged
PASS: Invalid colors rejected before truncating to BYTE
PASS: Set EMPTY erases final cell
PASS: ShiftDown copies all 192 bytes backward, discards old last row, clears top
PASS: AddTopRow fills 16 colors and preserves all lower rows
PASS: Repeated ShiftDown eventually empties grid (forward stores work after STD)
PASS: Empty grid returns exact boolean results
PASS: Each of 208 individual positions detected; danger only in rows 11 and 12
PASS: Scans treat any nonzero last byte as occupied
PASS: Real Con_Rand service returns bounded values and handles n <= 0
Grid tests: 31 checks, 0 failures
preview_smoke.cpp
PASS: preview frame text and danger line
PASS: cursor hidden while preview waits
PASS: ESC exits preview with code 0
PASS: cursor visible after shutdown
```

## Stage 2: resolution + existing regressions

```text
**********************************************************************
** Visual Studio 2026 Developer Command Prompt v18.10.2
** Copyright (c) 2026 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x86'
match.cpp
match_tests.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
PASS: Fill: exact color and four neighbours; different color and diagonal excluded
PASS: Fill: grid and score unchanged
PASS: Fill: already visited start is not counted again
PASS: Fill: mode 1 connects different colors and ignores color argument
PASS: Fill: all four corners terminate safely
PASS: Fill: invalid and empty starts preserve arrays and count
PASS: Fill: full-width mismatched color and invalid mode rejected
PASS: Fill: all 208 cells visited exactly once
PASS: Fill: top-level ESP unchanged after full-grid recursion
PASS: Fill: 118-cell one-wide serpentine path, no shortcuts or stack corruption
PASS: Resolve: exactly 3 pop, nonzero score gains 30, grid empty
PASS: Resolve: 2-match leaves disconnected bubbles and score unchanged
PASS: Resolve: entire component larger than 3 removed
PASS: Resolve: diagonal-only bubbles cannot form a match
PASS: Resolve: full 208-cell pop scores exactly 2080
PASS: Resolve: empty and invalid starts preserve grid, score, visited and popCount
PASS: Resolve: repeated calls clear stale marks and never double-score empty cells
Match tests: 17 checks, 0 failures
asm_test.cpp
MSVC x86 inline assembly: 7 + 5 = 12
asm_test.cpp
console.cpp
data.cpp
game.cpp
grid.cpp
main.cpp
match.cpp
score.cpp
shooter.cpp
Generating Code...
Built bin\AsmProof.exe and bin\BubbleShooter.exe for x86.
grid_tests.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
PASS: Init level 1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2: 6 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 3: 7 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 4: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 5: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 100: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2147483647: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 0: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -2147483648: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Get/Set every cell, including EMPTY and all five colors; no neighbor writes
PASS: Invalid coordinates return EMPTY and leave grid unchanged
PASS: Invalid colors rejected before truncating to BYTE
PASS: Set EMPTY erases final cell
PASS: ShiftDown copies all 192 bytes backward, discards old last row, clears top
PASS: AddTopRow fills 16 colors and preserves all lower rows
PASS: Repeated ShiftDown eventually empties grid (forward stores work after STD)
PASS: Empty grid returns exact boolean results
PASS: Each of 208 individual positions detected; danger only in rows 11 and 12
PASS: Scans treat any nonzero last byte as occupied
PASS: Real Con_Rand service returns bounded values and handles n <= 0
Grid tests: 31 checks, 0 failures
preview_smoke.cpp
PASS: preview frame text and danger line
PASS: cursor hidden while preview waits
PASS: ESC exits preview with code 0
PASS: cursor visible after shutdown
```

## Stage 3: floating removal + existing regressions

```text
**********************************************************************
** Visual Studio 2026 Developer Command Prompt v18.10.2
** Copyright (c) 2026 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x86'
match.cpp
match_tests.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
PASS: Fill: exact color and four neighbours; different color and diagonal excluded
PASS: Fill: grid and score unchanged
PASS: Fill: already visited start is not counted again
PASS: Fill: mode 1 connects different colors and ignores color argument
PASS: Fill: all four corners terminate safely
PASS: Fill: invalid and empty starts preserve arrays and count
PASS: Fill: full-width mismatched color and invalid mode rejected
PASS: Fill: all 208 cells visited exactly once
PASS: Fill: top-level ESP unchanged after full-grid recursion
PASS: Fill: 118-cell one-wide serpentine path, no shortcuts or stack corruption
PASS: Resolve: exactly 3 pop, nonzero score gains 30, grid empty
PASS: Resolve: 2-match leaves disconnected bubbles and score unchanged
PASS: Resolve: entire component larger than 3 removed
PASS: Resolve: diagonal-only bubbles cannot form a match
PASS: Resolve: full 208-cell pop scores exactly 2080
PASS: Resolve: empty and invalid starts preserve grid, score, visited and popCount
PASS: Resolve: repeated calls clear stale marks and never double-score empty cells
PASS: Floating: mixed-color chain to row 0 remains; popCount reused as connected count
PASS: Floating: pop 3 + drop 2 adds 70 to starting 100, no stale matched count
PASS: Floating: repeated resolve on removed cells never scores twice
PASS: Floating: popping support releases two bubbles while ceiling anchor remains
PASS: Floating: all nonempty top-row roots marked, overlapping roots not double-counted
PASS: Floating: next successful match clears old connectivity marks
PASS: Floating: diagonal contact does not attach an orphan to the ceiling
PASS: Floating: failed 2-match never invokes floating removal
Match tests: 25 checks, 0 failures
asm_test.cpp
MSVC x86 inline assembly: 7 + 5 = 12
asm_test.cpp
console.cpp
data.cpp
game.cpp
grid.cpp
main.cpp
match.cpp
score.cpp
shooter.cpp
Generating Code...
Built bin\AsmProof.exe and bin\BubbleShooter.exe for x86.
grid_tests.cpp
grid.cpp
data.cpp
console.cpp
Generating Code...
PASS: Init level 1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2: 6 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 3: 7 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 4: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 5: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 100: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2147483647: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 0: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -2147483648: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Get/Set every cell, including EMPTY and all five colors; no neighbor writes
PASS: Invalid coordinates return EMPTY and leave grid unchanged
PASS: Invalid colors rejected before truncating to BYTE
PASS: Set EMPTY erases final cell
PASS: ShiftDown copies all 192 bytes backward, discards old last row, clears top
PASS: AddTopRow fills 16 colors and preserves all lower rows
PASS: Repeated ShiftDown eventually empties grid (forward stores work after STD)
PASS: Empty grid returns exact boolean results
PASS: Each of 208 individual positions detected; danger only in rows 11 and 12
PASS: Scans treat any nonzero last byte as occupied
PASS: Real Con_Rand service returns bounded values and handles n <= 0
Grid tests: 31 checks, 0 failures
preview_smoke.cpp
PASS: preview frame text and danger line
PASS: cursor hidden while preview waits
PASS: ESC exits preview with code 0
PASS: cursor visible after shutdown
```

## Final execution of solution-built programs

```text
PASS: Fill: exact color and four neighbours; different color and diagonal excluded
PASS: Fill: grid and score unchanged
PASS: Fill: already visited start is not counted again
PASS: Fill: mode 1 connects different colors and ignores color argument
PASS: Fill: all four corners terminate safely
PASS: Fill: invalid and empty starts preserve arrays and count
PASS: Fill: full-width mismatched color and invalid mode rejected
PASS: Fill: all 208 cells visited exactly once
PASS: Fill: top-level ESP unchanged after full-grid recursion
PASS: Fill: 118-cell one-wide serpentine path, no shortcuts or stack corruption
PASS: Resolve: exactly 3 pop, nonzero score gains 30, grid empty
PASS: Resolve: 2-match leaves disconnected bubbles and score unchanged
PASS: Resolve: entire component larger than 3 removed
PASS: Resolve: diagonal-only bubbles cannot form a match
PASS: Resolve: full 208-cell pop scores exactly 2080
PASS: Resolve: empty and invalid starts preserve grid, score, visited and popCount
PASS: Resolve: repeated calls clear stale marks and never double-score empty cells
PASS: Floating: mixed-color chain to row 0 remains; popCount reused as connected count
PASS: Floating: pop 3 + drop 2 adds 70 to starting 100, no stale matched count
PASS: Floating: repeated resolve on removed cells never scores twice
PASS: Floating: popping support releases two bubbles while ceiling anchor remains
PASS: Floating: all nonempty top-row roots marked, overlapping roots not double-counted
PASS: Floating: next successful match clears old connectivity marks
PASS: Floating: diagonal contact does not attach an orphan to the ceiling
PASS: Floating: failed 2-match never invokes floating removal
Match tests: 25 checks, 0 failures
PASS: Init level 1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2: 6 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 3: 7 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 4: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 5: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 100: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 2147483647: 8 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level 0: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -1: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Init level -2147483648: 5 colored rows, rest empty, visited unchanged
PASS: Init repeatable with same CRT seed
PASS: Get/Set every cell, including EMPTY and all five colors; no neighbor writes
PASS: Invalid coordinates return EMPTY and leave grid unchanged
PASS: Invalid colors rejected before truncating to BYTE
PASS: Set EMPTY erases final cell
PASS: ShiftDown copies all 192 bytes backward, discards old last row, clears top
PASS: AddTopRow fills 16 colors and preserves all lower rows
PASS: Repeated ShiftDown eventually empties grid (forward stores work after STD)
PASS: Empty grid returns exact boolean results
PASS: Each of 208 individual positions detected; danger only in rows 11 and 12
PASS: Scans treat any nonzero last byte as occupied
PASS: Real Con_Rand service returns bounded values and handles n <= 0
Grid tests: 31 checks, 0 failures
PASS: preview frame text and danger line
PASS: cursor hidden while preview waits
PASS: ESC exits preview with code 0
PASS: cursor visible after shutdown
MSVC x86 inline assembly: 7 + 5 = 12
MatchTests PE: x86
GridTests PE: x86
BubbleShooter PE: x86
AsmProof PE: x86
PreviewSmoke PE: x86
```

