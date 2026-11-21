# Code walkthrough through the Match milestone

Ownership labels name the planned human maintainer; generation is disclosed in
AI_DISCLOSURE.md. All functions use cdecl. Asm_Add, the seven grid functions, and
the three Match functions contain inline assembly. Registers for C++ services are chosen by MSVC, with no handwritten
register use or guarantees beyond the calling convention.

## Shared interface: src/game.h

Owner: shared, Usman coordinates integration. Constants reproduce sections 7.1
and 7.3. COLS * ROWS is 208 bytes per BYTE array. Each grid cell is addressed
later by row * COLS + col. `extern` declares shared storage without allocating
another copy. The compiler guard rejects x64 and other compilers.

The following declarations form the shared interface. Grid functions are now
implemented below, as are Match functions; Game, Shooter, Score, and Num remain reserved only:

| Future function | Inputs | Planned output/purpose |
| --- | --- | --- |
| Game_Reset | None | Reset game globals |
| Game_Update | Globals, including keyBits | Update state machine |
| Shooter_Aim | Signed step | Change aimIndex within bounds |
| Shooter_Fire | Current aim and color globals | Begin shot |
| Shooter_Update | Shot globals | Advance and resolve movement |
| Grid_Init | Level global | Fill starting grid |
| Grid_Get | col, row | Integer cell value |
| Grid_Set | col, row, color | Store cell value |
| Grid_ShiftDown | Grid global | Move rows down |
| Grid_AddTopRow | Grid global | Fill row zero |
| Grid_CheckDanger | Grid global | Integer true/false |
| Grid_IsEmpty | Grid global | Integer true/false |
| Match_Fill | col, row, color, mode | Mark visited, update popCount |
| Match_Resolve | col, row | Resolve a landed match |
| Match_DropFloating | Grid global | Remove disconnected bubbles |
| Score_AddPop | Count | Add popped-bubble points |
| Score_AddDrop | Count | Add floating-bubble points |
| Score_UpdateLevel | Grid/level globals | Advance level when appropriate |
| Score_UpdateDrop | Whether a shot popped bubbles | Update ceiling counter |
| Num_ToString | Value, writable bufferAddress | Decimal text in caller's buffer |

## Shared storage: src/data.cpp

No functions. Defines every global listed in section 7.2 once. Grid, visited,
shot data, score, high score, key bits, pop count, and number buffer start zero.
Level starts at 1, aim at index 4, colors at 1, drop counter at 7, and state at
STATE_MENU. These are initial data, not gameplay initialization. The preview
does not run the state machine. Direction arrays exactly reproduce section 7.4;
positive DY means upward magnitude and is reserved for future movement.

## Console interface: src/console.h

Owner: Abdullah. Declares the implemented functions below. Reserved services
are declarations only: Con_DrawBubble(col,row,color) draws/erases a grid cell;
Con_DrawShot(cellX,cellY,color) draws/erases a shot; Con_DrawAim(index) draws the
aim; Con_DrawLauncher(curColor) draws the launcher; Con_ReadKeys() fills raw
keyBits; Con_Sleep(ms) delays; Con_Beep(freq,ms) plays sound;
Con_ShowScreen(which) draws a state screen. These reserved functions return void.
Con_Rand(n) is implemented in Milestone 2 and returns an integer as described below.

## Console implementation: src/console.cpp

Owner: Abdullah. Internal static globals save the output handle, original text
attributes, cursor properties, title, and setup status.

| Function | Inputs | Output and explanation |
| --- | --- | --- |
| Con_Init | None | Returns 1/0. Gets the console, saves appearance, ensures an 80x25 view and sufficient buffer, hides cursor, sets title, clears once. Restores saved appearance on setup failure. A success status is added to the suggested interface so main can report setup failure. |
| Con_Shutdown | Saved console globals | Void. Restores cursor visibility/size, text attributes, and title; moves cursor below preview. Safe after shutdown. Window size and erased contents are not restored. |
| Con_DrawText | x,y, text pointer, color attribute | Void. Moves cursor, selects attribute, writes ASCII text. Coordinates are zero based; caller supplies valid text and positions. |
| Con_DrawFrame | Constants | Void. Draws title, borders, labels, controls, static aim/launcher, and footer once. Grid area starts at (1,2); each reserved cell is two characters wide. Right border x=33, danger y=2+11=13, bottom y=17. Extra launcher lines sit below the 13 grid rows. HUD text is literal placeholder text, not number conversion. |
| Con_WaitForExit | Keyboard | Void. Blocks on _getch until ASCII ESC (27). Other keys do nothing. This temporary preview wait is separate from future raw-key reading and assembly input logic. |
| Con_Rand | Positive integer n | Returns rand() % n in 0..n-1; returns 0 for n <= 0 to avoid division by zero. Uses the C runtime pseudorandom sequence, with no handwritten registers. This previously declared service is the only console behavior added in Milestone 2. |

All these functions have no handwritten assembly registers. Con_DrawText assumes
successful initialization; failures during later drawing are not separately
reported. The clear happens once, so there is no redraw loop causing flicker.

## Entry point: src/main.cpp

Owner: Usman. `main()` takes no inputs, returns 0 normally or 1 on console setup
failure. It initializes, draws once, waits for ESC, and shuts down. No game
reset/update calls or game loop are present. No handwritten registers.

## Isolated proof: src/asm_test.cpp

Owner: Usman. Entire file is enabled only by BUBBLE_ASM_TEST. Thus wildcard game
builds have no second main. The separate AsmProof project enables that macro.

`Asm_Add(first, second)` accepts two small signed integers and returns their sum.
Only EAX is used explicitly; ESP and EBP are left unchanged. MSVC manages the
normal function stack frame. It is not a naked function.

| Instruction | Beginner explanation |
| --- | --- |
| `mov eax, first` | Copy the first input into EAX, a 32-bit CPU working register. MOV copies; it does not erase the input. |
| `add eax, second` | Add the second input to EAX. For 7 and 5, EAX becomes 12. ADD also updates arithmetic flags, which this test does not read. |
| `mov result, eax` | Copy EAX into the C++ local result. The C++ return statement then returns it normally. |

`__asm` and braces tell MSVC where assembly begins and ends; they are not CPU
instructions. The proof does not implement signed-overflow reporting.

The proof's `main()` takes no inputs, calls Asm_Add(7,5), prints its returned
value, and returns 0 if it is 12 or 1 otherwise. No handwritten registers here.
Compiler-generated prologues/epilogues are separate from the three source
instructions and are not counted as handwritten assembly.

## Reserved source modules

`game.cpp` and `shooter.cpp` belong to Usman; `score.cpp` belongs to Abdullah.
They contain owner/concept comments
only, with no functions, inputs, outputs, register use, or gameplay behavior.

## Build and reporting tools

`build.bat` compiles/runs the proof before compiling the preview; its failure
label returns a failing exit code. `BubbleShooter.sln` contains two separate
Debug|Win32 v145 console projects plus the isolated GridTests and MatchTests projects. The existing
two project files retain the user's VS 2026 configuration. `tools/ratio.ps1` has no named functions: it
accepts optional SourceDirectory and ReportPath, scans ordinary quoted C++
source line by line while tracking block comments and __asm braces, prints
per-file totals for src/ and tests/, and writes both production and combined
percentages as LF Markdown. It counts lines, not instructions emitted
by the compiler; wrapper braces count as C++ and comments never add assembly.

## Grid implementation: src/grid.cpp (Milestone 2)

Owner: Khizar Sheikh. Coordinates retain the shared interface's **(col, row)**
order. Columns 0..15 and rows 0..12 are valid. TOTAL_CELLS=208 and
SHIFT_CELLS=192 are local constants derived from COLS and ROWS; no shared header
or data definitions were changed. Grid routines touch grid only, not visited,
score, level, or other game state. They do not draw, score, match, or end a game.

| Function | Inputs / return | Explicit registers | Assembly logic |
| --- | --- | --- | --- |
| Grid_Init | Reads level; void | EAX/AL, EBX, ECX, EDI; ESP for call cleanup | CLD and REP STOSB clear all 208 bytes. Clamp the effective level to 1..4, add 4, then SHL by 4 to obtain 80/96/112/128 filled cells. Loop through those cells, call Con_Rand(5), add 1, and store the color. Valid levels follow min(4+level,8); nonpositive levels defensively use level-1 row count without changing level. Clamping before adding avoids overflow for INT_MAX. |
| Grid_Get | col,row; int color or EMPTY for invalid coordinates | EAX, ECX, EDX | Signed CMP/JL/JGE reject negative and too-large values before indexing. SHL row by 4 and ADD col produce row*16+col. LEA gets the grid address; MOVZX expands the BYTE to an unsigned integer. Invalid path returns 0. |
| Grid_Set | col,row,color; void | EAX, ECX/CL, EDX, ESI | Check coordinates and the full signed color range 0..5 before truncating anything. Calculate the same row-major index and store CL. Invalid calls do nothing; color 0 erases. |
| Grid_ShiftDown | Grid; void | EAX/AL, ECX, ESI, EDI; direction flag | Set ESI to index 191, EDI to 207, ECX to 192. STD makes REP MOVSB copy backward, so the overlapping destination never destroys unread source bytes. CLD immediately follows REP MOVSB. Clear the 16-byte top row with REP STOSB. Old row 12 is discarded; AddTopRow and danger checks are separate calls. |
| Grid_AddTopRow | Grid; void | EAX/AL, EBX, EDI; ESP for call cleanup | Loop exactly 16 times, calling Con_Rand(5) and storing its result plus 1 at successive row-zero addresses. All lower rows remain unchanged. |
| Grid_CheckDanger | Grid; int 1 if occupied, otherwise 0 | EAX, ECX, ESI | Start at index 176 (row 11). Scan exactly 32 bytes through index 207, returning 1 on the first nonzero value. Row 10 is not dangerous. |
| Grid_IsEmpty | Grid; int 1 if empty, otherwise 0 | EAX, ECX, ESI | Scan all 208 bytes. Start with result 1 and replace it with 0 if any nonzero byte is found. |

The routines' labels are local to their C++ functions and have distinct names.
Integer-returning routines save EAX into a C++ local and return it normally, so
warning 4035 suppression is unnecessary. MSVC generates normal stack frames and
saves/restores the callee-preserved EBX, ESI, and EDI registers when used. EBP is
never changed manually. Grid_Init and Grid_AddTopRow keep their pointer/count in
EDI/EBX across calls because cdecl preserves those registers. They do not rely
on EAX, ECX, or EDX surviving Con_Rand.

For Con_Rand(5), PUSH places the single 4-byte argument on the stack; CALL enters
the C++ service; ADD ESP,4 removes that argument afterward. The return value is
in EAX. The stack is balanced on every loop iteration. No call or return occurs
between STD and CLD. The test build enables /RTC1, including stack checks.

Instruction vocabulary: MOV copies data, MOVZX widens a byte with zero fill,
LEA gets an address, ADD adds values (or removes a stack argument via ESP),
SHL by 4 multiplies a checked nonnegative index by 16, XOR EAX,EAX sets zero,
CMP sets comparison flags, conditional jumps choose a path, JMP skips a path,
INC/DEC advance pointers/counts, and JNZ repeats while a count is nonzero.
REP repeats a string instruction ECX times. STOSB stores AL at EDI; MOVSB copies
the byte at ESI to EDI. Both advance pointers with CLD or decrement them with STD.
SUB is not needed for these algorithms; no unused instructions were added.

## Isolated test harnesses

Owner: Khizar. These files are outside src/, never enter the preview, and are
excluded from the specified src-only ratio but disclosed in the combined ratio.
The Grid and Preview harnesses use plain C++ with no handwritten assembly;
MatchTests additionally uses two MOV instructions to inspect ESP.

In tests/grid_tests.cpp:

| Function | Inputs / output | Purpose |
| --- | --- | --- |
| Check | Condition, description; void | Count each test result, print PASS/FAIL, accumulate failures. |
| TestInit | None; void | Test levels 1,2,3,4,5,100,INT_MAX,0,-1,INT_MIN. Validate every initialized/empty byte, visited preservation, unchanged level, and repeatability after resetting the CRT seed. |
| TestGetSet | None; void | Check every coordinate's stored value and unchanged neighbors, all supported colors/erase, invalid coordinates, and invalid full-width colors. |
| TestShiftAndTop | None; void | Compare a distinct-byte snapshot with the shifted grid, check empty row 0 and filled top row, preserve lower rows, and repeat shifts until empty. |
| TestScans | None; void | Place a single bubble at each of 208 positions to check exact empty/danger booleans, including both ends of danger rows and a nonzero invalid color byte. |
| TestRandom | None; void | Exercise the actual console service 1000 times, plus n=0 and n=-1. |
| main | None; exit 0/1 | Run all test groups and print checks/failures. |

In tests/preview_smoke.cpp, HasText(screen,x,y,expected) returns true when the
child console contains the expected ASCII text. Its main() launches the existing
preview in a hidden separate console, waits for expected frame text, checks the
hidden cursor, sends ESC, waits for exit 0, checks the restored visible cursor,
and closes handles. It returns 0 only if all four checks pass; timeout/failure
cleans up its own child. This checks actual console state without changing main.
It does not replace human visual inspection of fonts, sizing, or colors.

tests/build_grid_tests.bat builds/runs both harnesses using x86 MSVC. Run the
normal build first so PreviewSmoke tests a current bin/BubbleShooter.exe.
GridTests.vcxproj also lets the team debug the grid harness in VS 2026 with
Debug|Win32, v145, /W4, warnings as errors, and runtime checks.

## Match implementation: src/match.cpp

Owner: Khizar Sheikh. This module implements actual recursion and four-direction
connectivity. It does not integrate shooting, menus, drawing, or other gameplay.
All algorithmic work is in __asm blocks. TOTAL_CELLS is a file-local constant
derived from COLS * ROWS, not a new shared declaration.

Verified interfaces (all x86 cdecl):

```cpp
void __cdecl Match_Fill(int col, int row, int color, int mode);
void __cdecl Match_Resolve(int col, int row);
void __cdecl Match_DropFloating();
int __cdecl Grid_Get(int col, int row);
void __cdecl Grid_Set(int col, int row, int color);
void __cdecl Score_AddPop(int count);
void __cdecl Score_AddDrop(int count);
void __cdecl Score_UpdateLevel();
void __cdecl Score_UpdateDrop(int popped);
```

Grid and Match use **col before row**. Match returns void, never counts or points.
The Score declarations still have no definitions in score.cpp and are not called
here. Match updates the existing score in assembly as requested. Its shared data
is grid, visited, popCount, and score. It does not modify level, highScore,
shotsUntilDrop, or shot state. Grid_Init separately reads level.

| Function | Inputs / outputs | Explicit registers and frame data | Logic |
| --- | --- | --- | --- |
| Match_Fill | col,row,color,mode; void. Reads grid; sets visited and adds to popCount. Never clears scratch itself or changes grid/score. | EAX, ECX, EDX; ESP for four argument pushes and cleanup. Parameters are reloaded by name from each frame. | Signed JL/JGE bounds checks precede index = (row << 4) + col. Reject visited or empty cells. Mode 0 requires exact full-width color equality; mode 1 accepts any nonempty color. Invalid modes do nothing. Mark first, increment count, then call itself UP, DOWN, LEFT, RIGHT. Each child receives four ints and is followed by ADD ESP,16. |
| Match_Resolve | col,row; void. Pops matching bubbles and then floating bubbles, adding both point categories once. | EAX, EBX, ECX, ESI, EDI; ESP for calls. Local matchedCount at [ebp-4] in the reviewed build. | Validate selected cell before modifying any state. CLD/REP STOSB clear visited; popCount=0. Run Fill mode 0. If count<3, leave grid/score unchanged and do not call DropFloating. Otherwise save count, clear visited grid cells, multiply saved count by 10 using IMUL, add score, then call DropFloating once. |
| Match_DropFloating | No parameters; void. Requires a preceding successful pop. Removes unanchored bubbles and adds 20 each. | EAX, ECX, ESI, EDI; ESP for calls. Local topCol at [ebp-4] in the reviewed build. | Clear scratch with CLD/REP STOSB, popCount=0. Loop all 16 top-row cells and call Fill(mode=1) for each nonempty root. Keep topCol in the frame across calls. Scan TOTAL_CELLS and remove only nonempty cells with visited=0, adding POINTS_DROP per removal. |

Recursion means a function calls itself to solve the same smaller problem: visit
one neighboring cell. Conditional jumps are its stopping rules. Without the
visited array, two adjacent cells could repeatedly call each other forever.
Marking before calling neighbors makes each occupied cell count at most once.
Mode 0 asks "which touching bubbles have this color?" Mode 1 asks "which bubbles
have any four-direction path to the ceiling?", regardless of color.

Worked example: put color 2 at (0,0), (1,0), and (0,1), with everything else
empty. Resolve(0,0) clears scratch and calls Fill(0,0,2,0). The starting cell is
marked; its downward and rightward calls eventually mark the other two. Upward,
leftward, empty, and already visited calls stop safely. popCount becomes 3.
Resolve saves 3, removes all three visited cells, and awards 3*10=30 points.
DropFloating then clears scratch; with no bubbles left it counts zero ceiling
connections and removes nothing. Starting score 100 ends at 130. popCount ends
at 0, illustrating why the matched count must be saved before the second search.

Counts versus points: after a direct Fill, popCount is the number visited since
the caller last reset it. After a valid failed Resolve, it is the small matching
group size. After a successful Resolve, DropFloating has reused it as the number
of remaining top-connected cells. It is never a reliable returned removal count.
Invalid or empty Resolve calls preserve grid, score, visited, and popCount.

### Stack frames and the actual compiled layout

Conventional 32-bit cdecl frame for Match_Fill, high addresses at the top:

```text
higher addresses
[ebp+20] = mode
[ebp+16] = color
[ebp+12] = row
[ebp+8]  = col
[ebp+4]  = return address
[ebp]    = saved EBP       <- EBP stays fixed within this call
lower addresses           <- stack grows downward as ESP decreases
```

This is verified, not assumed: the v145 compiler listing (compiler
19.51.36260.0, /Od /RTC1) declares _col$=8, _row$=12, _color$=16, _mode$=20 and
emits PUSH EBP / MOV EBP,ESP for Match_Fill. It has no local storage or manually
saved working registers. At the end the compiler checks ESP against EBP,
restores EBP with POP, and emits RET 0. The listing is an ignored text artifact
at obj/MatchTests/match_listing.txt, generated by the test script; it is never
assembled as an external .asm source. Other build flags can change details, so
inspect the current Disassembly view rather than relying on offsets blindly.

PUSH subtracts 4 from ESP and stores a 32-bit value. For each child the source
pushes mode, color, new row, new col, in that order. CALL then pushes the address
of the next instruction and jumps into Match_Fill. Its compiler prologue saves
the parent's EBP and sets a new EBP for the child. Each frame therefore retains
its own coordinates even while the same function runs recursively.

RET takes the saved return address off the stack and resumes the caller. It does
not remove the four cdecl arguments; ADD ESP,16 does that afterward. POP reads
the value at ESP and then adds 4 to ESP. No explicit POP is needed in our Fill
body because no parent register values are kept across calls; we reload the
parameters. MSVC uses POP in its epilogue to restore EBP and any callee-preserved
registers used by the other functions. An unbalanced ESP could make later data
look like an argument, saved register, or return address and corrupt execution.

The reviewed Resolve frame reserves matchedCount at EBP-4 and saves EBX/ESI/EDI;
DropFloating reserves topCol at EBP-4 and saves ESI/EDI. MSVC generates these
saves/restores. No code manually changes EBP. Every one of the four recursive
call sites, plus the Resolve and DropFloating Fill calls, has exactly four
argument PUSH instructions and ADD ESP,16. The zero-argument DropFloating call
needs no argument cleanup. /RTC1 checks every Fill return; top-level ESP equality
is additional evidence, not the only stack check.

The snake test uses seven full even rows and six odd-row connectors alternating
between col 15 and col 0. It contains 7*16+6=118 cells. Empty separator-row cells
prevent side shortcuts, so walking from (0,0) requires a chain of 118 marked
recursive frames before unwinding. Rejected neighbor calls briefly add frames
too. The full-grid test separately visits all 208 cells exactly once.

### Shooter integration contract (instructions only)

After a bubble lands, the future shooter should store it with
Grid_Set(col,row,color), save score if it needs to detect a successful pop, and
call Match_Resolve(col,row) once. Match already removes and scores floaters after
a successful pop. Do not call Match_DropFloating again and do not award either
pop or floating points through Score_AddPop/Score_AddDrop a second time.

With this void interface, compare score before/after the synchronous Resolve to
detect a successful pop for the later ceiling counter. Do not use popCount as
the matched count after Resolve. There are no removal-count return values.
Continue with the planned ceiling/danger/level/HUD work in its own milestone.
No shooter, score, state-machine, sound, or rendering code was added here.
game.h changed comments only: signatures, constants, and data declarations remain
identical. Optional beep feedback remains deferred with the sound services.

### MatchTests harness

tests/match_tests.cpp is deterministic and writes grid[] directly. Check records
a condition/description and updates totals; Reset clears grid, visited, popCount,
score and related test state; VisitedSum returns the sum of marker bytes;
Occupied returns the number of nonempty cells. TestFill exercises both modes,
guards, corners, visited reuse, the full grid, the 118-cell path, and two MOV
instructions reading ESP around a normal C++ call. TestResolve covers pop sizes,
no-match behavior, invalid/empty inputs, full-grid points, and repeated calls.
TestFloating checks mixed-color anchors, support removal, exact combined scores,
multiple roots, stale scratch, and no-match floaters. These helpers take no
arguments except Check(condition,description); only the sum/count helpers and
main return values. main returns 0 when all 25 checks pass and 1 otherwise.
MSVC chooses helper registers; TestFill explicitly reads ESP without changing it.
