# Milestone 1 code walkthrough

Ownership labels name the planned human maintainer; generation is disclosed in
AI_DISCLOSURE.md. All functions use cdecl. Only Asm_Add contains handwritten
assembly. Registers for every C++ function are chosen by MSVC, with no handwritten
register use or guarantees beyond the calling convention.

## Shared interface: src/game.h

Owner: shared, Usman coordinates integration. Constants reproduce sections 7.1
and 7.3. COLS * ROWS is 208 bytes per BYTE array. Each grid cell is addressed
later by row * COLS + col. `extern` declares shared storage without allocating
another copy. The compiler guard rejects x64 and other compilers.

The following declarations reserve interfaces, with no implementation, register
use, or runtime output in Milestone 1:

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
keyBits; Con_Sleep(ms) delays; Con_Rand(n) returns 0..n-1; Con_Beep(freq,ms)
plays sound; Con_ShowScreen(which) draws a state screen. Except Con_Rand, these
return void. None has an assembly body or register use yet.

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

`game.cpp` and `shooter.cpp` belong to Usman; `grid.cpp` and `match.cpp` belong to
Khizar; `score.cpp` belongs to Abdullah. They contain owner/concept comments
only, with no functions, inputs, outputs, register use, or gameplay behavior.

## Build and reporting tools

`build.bat` compiles/runs the proof before compiling the preview; its failure
label returns a failing exit code. `BubbleShooter.sln` contains two separate
Debug|Win32 v143 console projects. `tools/ratio.ps1` has no named functions: it
accepts optional SourceDirectory and ReportPath, scans ordinary quoted C++
source line by line while tracking block comments and __asm braces, prints
per-file totals, and writes Markdown. It counts lines, not instructions emitted
by the compiler; wrapper braces count as C++ and comments never add assembly.
