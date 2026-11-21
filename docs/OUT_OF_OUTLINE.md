# Concepts outside the lab outline

## Win32 console API

**What:** Windows operating-system functions from windows.h for the console.

**Why:** The lab instruction set cannot directly move the Windows text cursor
or select console text colors.

**How:** GetStdHandle finds the output screen. GetConsoleScreenBufferInfo,
GetConsoleCursorInfo, and GetConsoleTitleA save appearance. Buffer/window size
calls reserve a visible 80x25 region. FillConsoleOutputCharacterA and
FillConsoleOutputAttribute clear it once. SetConsoleCursorInfo hides the cursor.
SetConsoleCursorPosition selects a text coordinate, SetConsoleTextAttribute
selects a color, and WriteConsoleA writes the ASCII text. Shutdown restores the
cursor properties, text attributes, and title. It does not restore erased text
or resize the window back.

**Where:** src/console.cpp: Con_Init, Con_Shutdown, Con_DrawText. The source has
a comment referring to this section above the console services.

**Simpler alternative:** printf can print sequential lines, but cannot provide
the cursor/color services needed for later cell updates. Win32 is the specified
replacement for Irvine32 drawing services. No extra library is installed.

## Console key wait

**What:** MSVC's _getch from conio.h reads a key without waiting for Enter.

**Why:** The static preview needs a clean exit while gameplay input is deferred.

**How:** Con_WaitForExit blocks until a key arrives and repeats unless its value
is 27 (ESC). It uses no polling loop or CPU-heavy redraw. Extended keys may
produce two reads and are ignored.

**Where:** src/console.cpp: Con_WaitForExit, with a section-reference comment.

**Simpler alternative:** getchar waits for Enter with normal console input. It
does not provide the requested immediate exit key. This temporary service will
not replace the specified future Con_ReadKeys/GetAsyncKeyState input scheme.

## Fixed-point data

**What:** Integers store scaled positions/directions: 256 units mean one cell.

**Why:** Future shots need smaller steps than a whole cell without FPU or SSE.

**How:** A straight-up direction has DX=0 and DY=256. The stored positive DY is
an upward magnitude; later shot code will negate it for screen movement. All
nine pairs are precomputed constants from section 7.4; no trigonometry or
movement runs in this milestone. Later arithmetic uses allowed shifts/adds.

**Where:** src/game.h SUBPIX and src/data.cpp angleDX/angleDY, with a reference
comment next to the tables. shotX/Y and shotDX/DY are reserved storage.

**Simpler alternative:** Whole-cell motion cannot represent the required nine
directions smoothly. Floating point is disallowed. Scaling integers is simple
and uses the course's ordinary integer instructions.

No file persistence, high-score file I/O, recursion fallback, or other extra
runtime concepts have been implemented. The PowerShell ratio scanner and batch
build are development tools, not game assembly topics.

## Pseudorandom colors

**What:** The basic C runtime rand() function generates a sequence of integers.

**Why:** The specified grid initialization and new-row service need colors 1..5.

**How:** Con_Rand(n) returns rand() % n for n>0, giving 0..n-1. The assembly caller
adds 1 for a bubble color. Nonpositive n returns 0 instead of dividing by zero.
This is simple pseudorandomness, not security randomness or a guarantee of equal
frequencies. Production seeding has not been added: the CRT's default sequence
repeats across fresh processes. The isolated tests explicitly use srand with
fixed seeds to make failures reproducible. Later startup integration can seed it.

**Where:** src/console.cpp, Con_Rand (with a reference comment); callers are
src/grid.cpp, Grid_Init and Grid_AddTopRow. tests/grid_tests.cpp seeds tests only.

**Simpler alternative:** A fixed color would not meet the random-color
requirement. A custom assembly generator adds logic that is unnecessary when
the specification already allows random numbers as a C++ service.

## Preview regression harness

**What:** Additional Win32 process and console calls used only by an isolated test.

**Why:** Verify that adding grid functions preserves the real preview's frame
and ESC exit without changing Usman's main.cpp or requiring manual input.

**How:** CreateProcess starts bin/BubbleShooter.exe with a hidden new console.
FreeConsole/AttachConsole attach only the test process to that child console.
CreateFile opens CONOUT$/CONIN$; ReadConsoleOutputCharacter checks frame text.
WriteConsoleInput supplies an ESC key event. WaitForSingleObject and
GetExitCodeProcess confirm exit. Cursor queries check visibility before/after.
Sleep bounds polling intervals. CloseHandle releases resources. On failure,
TerminateProcess cleans up only this test's own child process.

**Where:** tests/preview_smoke.cpp, HasText and main, with a reference comment.
These calls do not enter the game executable or its code ratio.

**Simpler alternative:** Human inspection is useful and remains requested. This
small automated check also provides a repeatable test of actual console output
and exit behavior, rather than assuming a successful build means it ran.
