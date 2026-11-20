# Code walkthrough through Milestone 2

Ownership labels name the planned human maintainer; generation is disclosed in
AI_DISCLOSURE.md. All functions use cdecl. Asm_Add and the seven grid functions
contain inline assembly. Registers for every C++ function are chosen by MSVC, with no handwritten
register use or guarantees beyond the calling convention.

## Shared interface: src/game.h

Owner: shared, Usman coordinates integration. Constants reproduce sections 7.1
and 7.3. COLS * ROWS is 208 bytes per BYTE array. Each grid cell is addressed
later by row * COLS + col. `extern` declares shared storage without allocating
another copy. The compiler guard rejects x64 and other compilers.

The following declarations form the shared interface. Grid functions are now
implemented below; Game, Shooter, Match, Score, and Num remain reserved only:

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

`game.cpp` and `shooter.cpp` belong to Usman; `match.cpp` belongs to
Khizar; `score.cpp` belongs to Abdullah. They contain owner/concept comments
only, with no functions, inputs, outputs, register use, or gameplay behavior.

## Build and reporting tools

`build.bat` compiles/runs the proof before compiling the preview; its failure
label returns a failing exit code. `BubbleShooter.sln` contains two separate
Debug|Win32 v145 console projects plus the isolated GridTests project. The existing
two project files retain the user's VS 2026 configuration. `tools/ratio.ps1` has no named functions: it
accepts optional SourceDirectory and ReportPath, scans ordinary quoted C++
source line by line while tracking block comments and __asm braces, prints
per-file totals, and writes Markdown. It counts lines, not instructions emitted
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
excluded from the specified src-only ratio. They use plain C++ and no handwritten
assembly registers; MSVC handles their registers and stack frames.

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
