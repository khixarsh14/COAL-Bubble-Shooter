# CODEX_CONTEXT.md — Bubble Shooter in x86 Inline Assembly (COAL Lab Project)

> Read this whole file before writing any code. It is the single source of truth for the project.
> You (Codex) are the **only** coding agent on this project. The human is **Khizar Sheikh**, who will build, run and test what you write, and paste back errors or screenshots.

---

## 1. What this project is

A **console-based Bubble Shooter game for Windows**, written for the **EL-2003 Computer Organization & Assembly Language (COAL) Lab**, FAST-NUCES Karachi, Fall 2026.

- The player controls a launcher at the bottom of the screen, aims with the arrow keys, and shoots colored bubbles at a grid of bubbles above.
- When 3 or more bubbles of the same color touch, they pop. Bubbles left hanging (not connected to the top) fall and are removed.
- The game has score, level, high score, a next-bubble preview, a ceiling that drops new rows, pause, menu, and game over.

**Team:** Usman Khan (25K-0965, group leader), Abdullah bin Faheem (25K-0603), Khizar Sheikh (25K-0641).
**Lab instructor:** Mr. Abdullah Shaikh.
**Deadlines:** final report, source code and demo + viva on **23 November 2026**. The viva is individual: every member must be able to explain every line of code.

---

## 2. HARD RULES (never break these)

1. **70% / 30% code ratio.** The instructor approved by email: *"Project should be 70% Assembly + 30% any other programming language."*
   - At least **70% of the code lines must be x86 assembly**, written as **inline assembly** (`__asm` blocks) inside C++ functions.
   - At most **30%** is C++ (console drawing, keyboard reading, timing, sound, program start-up, global data definitions).
   - Aim for **72–75% assembly** so there is a safety margin. See section 11 for how to measure and prove it.
2. **Inline assembly only.** Use MSVC `__asm { ... }` blocks. Do **not** use external `.asm` files for the main logic, and do not use another assembler.
3. **32-bit (x86) only.** MSVC inline assembly does not work on x64, GCC, MinGW or Clang. Build with the MSVC `cl` compiler targeting **x86 / Win32**.
4. **No external libraries.** No raylib, SDL, SFML, ncurses, STL-heavy code, or third-party packages. Only the C++ standard library basics and the Win32 console API (`windows.h`).
5. **Do not overcomplicate the assembly logic.** This is a student project that every member must explain in a viva.
   - Prefer the simplest algorithm that works.
   - Short procedures, clear names, one job each.
   - No clever bit tricks, no self-modifying code, no obscure instructions.
   - If there is a simple way and a fancy way, use the simple way.
6. **Stay inside the course outline** (section 3). Use only the assembly topics that are taught in the course. If a feature truly needs something outside the outline, follow the rules in section 4 and explain it clearly.
7. **No floating point.** FPU and SSE are not in the course. Use integer and fixed-point math (section 7.4).
8. **Heavy comments in the assembly.** Every `__asm` block gets a header comment saying what it does, and each non-obvious instruction gets a short comment. The team must be able to read it and explain it.
9. **Keep C++ minimal and plain.** Plain functions and global variables. No classes, templates, lambdas, STL containers, or exceptions.
10. **Always leave the project in a buildable state.** Work in small steps. After each step, the project must compile.

---

## 3. Course outline — the allowed assembly toolbox

Course: *Computer Organization & Assembly Language Lab (EL-2003)*. Textbook: Irvine, *Assembly Language for Intel-Based Computers*, 7th ed. Weekly topics:

| Week | Topics |
| --- | --- |
| 1 | Introduction to assembly, Visual Studio configuration |
| 2 | Debugging, basic elements of assembly, defining data, intrinsic data types, data definition statements |
| 3 | Data initialization, multiple initialization, string initialization |
| 4 | Data transfer instructions, instructions and flags |
| 5 | Data-related operators and directives, indirect addressing |
| 7 | Arrays and loops, built-in procedures |
| 9 | Stack operations, procedures, Boolean and conditional jumps |
| 10 | Conditional processing |
| 12 | Shift and rotate, multiplication and division, extended addition and subtraction |
| 13 | Stack frames, recursion, INVOKE, ADDR, PROC, PROTO directives |
| 14 | String handling instructions, two-dimensional arrays |
| 15 | Inline assembly |

**You may freely use:** `MOV, MOVZX, MOVSX, XCHG, LEA, ADD, SUB, INC, DEC, NEG, MUL, IMUL, DIV, IDIV, CMP, TEST, AND, OR, XOR, NOT, SHL, SHR, SAL, SAR, ROL, ROR, JMP, JE/JZ, JNE/JNZ, JL, JLE, JG, JGE, JB, JA, JBE, JAE, LOOP, PUSH, POP, CALL, RET, CLD, STD, REP MOVSB/MOVSD, REP STOSB/STOSD, LODSB, STOSB, CMPSB, SCASB`, indirect addressing (`[esi]`, `[esi+eax]`, `[esi+eax*4]`), 2D array indexing, stack frames, recursion, and `__asm` blocks.

**Concept mapping note:** `PROC`, `PROTO`, `INVOKE`, `ADDR` are MASM directives and cannot be used inside inline assembly. In this project, each C++ function whose body is an `__asm` block plays the role of a procedure with a stack frame. Explain this one-to-one mapping in the README (section 12).

**Avoid (not in the outline):** FPU (`fld`, `fstp`, ...), SSE/AVX, `CPUID`, `RDTSC`, jump tables via computed `JMP`, MASM macros/`.IF` directives, and anything exotic.

---

## 4. Rule for anything OUTSIDE the course outline

Some things in this project cannot be avoided but are not in the outline. These are expected:

- **Win32 console API calls** (cursor position, text color, key state, sleep, beep) — C++ side
- **Fixed-point arithmetic** — assembly side, but it only uses shifts, multiplies and adds
- **File I/O for the high score** (optional) — C++ side

For **each** such concept, and for anything else you realize is outside the outline while coding, you must:

1. Add a section to `docs/OUT_OF_OUTLINE.md` with: **What it is**, **Why this project needs it**, **How it works in simple words** (examples allowed), **Where it is used in the code** (file and function), and **A simpler alternative, if any, and why it wasn't chosen**.
2. Add a short comment in the code next to its use, pointing to that section.
3. Mention it in your reply to the user at the end of that step.

Write these explanations for a CS student who has just finished the assembly lab. Use plain English.

---

## 5. Game specification

### 5.1 Screen layout (console mock-up from the proposal)

```
 BUBBLE SHOOTER     x86 Assembly (inline __asm + C++)
+--------------------------------+
| O O O O O O O O O O O O O O O O |      SCORE : 0450
| O O O O O O O O O O O O O O O O |      LEVEL : 2
| O O O O O O O O O O O O O O O O |      HIGH  : 1200
| O O O O O O O O O O O O O O O O |
| O O   O O O     O O   O O       |      NEXT    : O
|   O     O   O     O O           |      DROP IN : 3 shots
|                                  |
|                                  |      CONTROLS
|                                  |      <- / ->  aim
|                                  |      SPACE    shoot
|- - - - DANGER LINE - - - - - - - |      P        pause
|                                  |      ESC      menu
|              ^                   |
|             [O]                  |
+--------------------------------+
 Match 3 or more bubbles of the same color to pop them!
```

- Each bubble is drawn as **2 characters wide** (`O ` in its color) so the cells look roughly square in the console.
- Colored with console text attributes. Five bubble colors (use clearly different console colors, e.g. red, green, blue, yellow, magenta).
- Use plain ASCII box characters (`+ - |`) so it works in any console font.

### 5.2 Grid

- **COLS = 16**, **ROWS = 13** (rows 0..12), stored as a 1D `BYTE grid[COLS * ROWS]` in row-major order. Index = `row * COLS + col`.
- Cell values: `0` = empty, `1..5` = bubble colors.
- **Danger row = 11.** If any bubble ends up on row 11 or lower after a shot is resolved, the game is over.
- **Plain rectangular grid with 4-neighbour connectivity** (up, down, left, right). **Do not** implement the hexagonal/offset layout. Keep it simple.

### 5.3 Controls

| Key | Action |
| --- | --- |
| Left / Right arrow | Rotate aim one step |
| Space | Fire the current bubble (only when no bubble is in flight) |
| P | Pause / resume |
| ESC | Go back to the main menu |
| Menu keys `1` / `2` / `3` | Start game / How to play / Exit |
| Any key on game-over screen | Return to menu |

### 5.4 Core rules

1. The launcher holds a **current bubble** and shows a **next bubble** (random colors 1..5).
2. The aim has **9 fixed angles**: -60°, -45°, -30°, -15°, 0°, +15°, +30°, +45°, +60° (index 0..8, start at 4 = straight up).
3. On Space, the current bubble flies along the aim direction. It **bounces** off the left and right walls.
4. The bubble stops when it **hits the ceiling (row 0 is top)** or **touches an existing bubble**. It then **snaps** into the last empty cell it passed through.
5. **Match check:** flood-fill from the snapped cell over same-colored neighbours. If the group has **3 or more**, remove it.
6. **Floating check:** after any pop, every remaining bubble that is **not connected to row 0** (through any colors) falls and is removed.
7. **Score:** 10 points per popped bubble, 20 per floating bubble dropped.
8. **Level:** when the grid becomes empty, go to the next level. A new level starts with `4 + level` filled rows (maximum 8 rows) of random colors.
9. **Ceiling drop:** every `N` shots with no pop, all rows shift down by one and a new random top row is added. `N = 8 - level`, minimum 3. The side panel shows "DROP IN: n shots". A shot that pops bubbles resets the counter.
10. **Game over:** a bubble reaches the danger row, or the ceiling drop pushes bubbles there. Show a GAME OVER message with the final score, update the high score, and return to the menu on a key press.
11. **High score** is kept for the session in memory. Saving it to a file is an **optional extra** (section 9).

---

## 6. Architecture — who does what

The design is "assembly drives, C++ serves":

- **Assembly (inline `__asm`) = the brain.** The game loop's update logic (state machine), input interpretation, aiming, shot movement, collision, snapping, grid operations, flood-fill, floating-bubble removal, scoring, level logic, number-to-text conversion, game-over checks.
- **C++ = the hands and eyes.** Start-up, reading raw key states, drawing characters at positions in colors, sleeping between frames, beeping, random numbers, and holding the global data arrays.

Assembly calls the C++ service functions using the normal C calling convention (`cdecl`: push arguments right to left, `call`, then `add esp, n`; the result comes back in `EAX`).

### 6.1 Suggested file layout

```
BubbleShooter/
  build.bat                  build with MSVC x86 (see section 10)
  README.md                  see section 12
  CODEX_CONTEXT.md           this file
  docs/
    CODE_WALKTHROUGH.md      explanation of every module for the viva
    OUT_OF_OUTLINE.md        see section 4
    AI_DISCLOSURE.md         see section 12
    TESTING.md               test checklist and results
    RATIO.md                 output of the ratio tool
  tools/
    ratio.ps1                counts assembly vs C++ lines
  src/
    game.h                   constants, extern globals, function declarations
    data.cpp                 global data: grid, visited, angle tables, state variables   (C++)
    console.cpp / console.h  Win32 console wrapper                                        (C++)
    main.cpp                 entry point and frame loop                                  (C++)
    game.cpp                 game state machine, input, menu/pause/game-over logic       (ASM)
    shooter.cpp              aim, fire, shot movement, wall bounce, snapping              (ASM)
    grid.cpp                 grid init, get/set, shift down, add row, danger check        (ASM)
    match.cpp                flood-fill, pop, floating-bubble removal                     (ASM)
    score.cpp                score, level, drop counter, number-to-string                 (ASM)
```

Put a header comment at the top of each source file with the **owner** and the **concepts it uses**.

### 6.2 Work split between the three members

The code should be organized so each member owns a clear part, and each member owns both assembly and a little C++.

| Member | Owns |
| --- | --- |
| **Usman Khan** (leader) | `game.cpp` (state machine, main update, input to aim, menu/pause/game-over states), `shooter.cpp` (aim table use, fire, movement, wall bounce, snapping, next-bubble handling), `main.cpp` (frame loop), final integration |
| **Khizar Sheikh** | `grid.cpp` (init with random bubbles, get/set, shift rows down with string instructions, add top row, danger-row check, empty-grid check), `match.cpp` (recursive flood-fill, pop group, floating-bubble removal) |
| **Abdullah bin Faheem** | `console.cpp/.h` (the whole C++ wrapper: drawing, colors, key reading, timing, sound, random), `score.cpp` (score, level, drop counter, number-to-string using `DIV`), HUD drawing calls, testing docs |

Mark the owner at the top of each file so the viva questions line up with the code.

---

## 7. Detailed technical design

### 7.1 Constants (in `game.h`)

```
COLS = 16        ROWS = 13        DANGER_ROW = 11
EMPTY = 0        NUM_COLORS = 5   NUM_ANGLES = 9
MIN_MATCH = 3    POINTS_POP = 10  POINTS_DROP = 20
SUBPIX = 256     (fixed-point: 256 units = 1 grid cell)
```

States: `STATE_MENU = 0, STATE_PLAYING = 1, STATE_PAUSED = 2, STATE_GAMEOVER = 3, STATE_HELP = 4, STATE_EXIT = 5`.

### 7.2 Global data (in `data.cpp`, C++ — inline asm cannot declare data)

```
BYTE grid[COLS*ROWS];          // 0 empty, 1..5 color
BYTE visited[COLS*ROWS];       // scratch for flood-fill
int  angleDX[9], angleDY[9];   // fixed-point direction table, see 7.4
int  aimIndex;                 // 0..8
int  curColor, nextColor;      // current and next bubble color
int  shotActive;               // 0/1: is a bubble flying
int  shotX, shotY;             // fixed-point position (256 = 1 cell)
int  shotDX, shotDY;           // fixed-point velocity per step
int  lastCol, lastRow;         // last empty cell the shot passed through
int  score, level, highScore;
int  shotsUntilDrop;
int  gameState;
int  keyBits;                  // bit mask from the keyboard reader
int  popCount;                 // scratch result of flood-fill
char numBuf[12];               // text buffer for number-to-string
```

### 7.3 Key bits (filled by C++ `Con_ReadKeys`, interpreted by assembly)

```
KEY_LEFT = 1, KEY_RIGHT = 2, KEY_SPACE = 4, KEY_P = 8, KEY_ESC = 16,
KEY_1 = 32, KEY_2 = 64, KEY_3 = 128, KEY_ANY = 256
```

The assembly must use `TEST` / `AND` / `CMP` to read these bits. The C++ side only reports raw key states and does **not** decide what they mean. Use an "edge" check (key newly pressed since last frame) for Space, P, ESC and menu keys, so one key press does not repeat.

### 7.4 Aim table and fixed-point movement

No floating point. Precompute the direction vectors for the 9 angles, scaled by 256, and store them in `angleDX` / `angleDY` (sin and cos of the angle times 256, rounded):

| Index | Angle | dx | dy (upwards, stored positive) |
| --- | --- | --- | --- |
| 0 | -60° | -222 | 128 |
| 1 | -45° | -181 | 181 |
| 2 | -30° | -128 | 222 |
| 3 | -15° | -66 | 247 |
| 4 | 0° | 0 | 256 |
| 5 | +15° | 66 | 247 |
| 6 | +30° | 128 | 222 |
| 7 | +45° | 181 | 181 |
| 8 | +60° | 222 | 128 |

On fire: copy `angleDX[aimIndex]` and `-angleDY[aimIndex]` (negative because screen Y grows downward) into `shotDX` / `shotDY`. Position is kept in 1/256-cell units. To get the grid cell from a position, **shift right by 8** (`SAR 8`). Each frame, move the shot in **several small sub-steps** (for example 4 sub-steps, each moving `shotDX/4`, `shotDY/4` using `SAR 2`) and check collisions after every sub-step, so the bubble cannot jump through another bubble.

Explain fixed-point math in `docs/OUT_OF_OUTLINE.md` (it uses only shifts, adds and multiplies, but the idea of "scaling by 256" is not in the outline).

### 7.5 Shot movement, bounce, snap (simple version)

For each sub-step:

1. `shotX += shotDX/4`, `shotY += shotDY/4`.
2. **Wall bounce:** if `shotX < 0` or `shotX >= COLS*256`, then `shotDX = -shotDX` (`NEG`) and move the shot back inside.
3. Convert to a cell: `col = shotX >> 8`, `row = shotY >> 8`.
4. **Hit test:** if `row <= 0`, or `grid[row][col] != EMPTY`, the shot has landed.
5. Otherwise, remember `(col,row)` as `(lastCol,lastRow)` (the last empty cell visited) and continue.
6. **On landing:** place the bubble at `(lastCol,lastRow)` (if the hit was the ceiling, use `row = 0`). Then run the match check (section 7.6), the floating check, scoring, the ceiling-drop counter, and the game-over check, in that order. Then load the next bubble.

### 7.6 Flood-fill (the recursion showpiece — Khizar's module)

`Match_Fill(col, row, color, mode)` is a **recursive** function written as an `__asm` block that **calls itself**.

- `mode 0` = match same color; `mode 1` = connectivity (any non-empty bubble, used for the floating check).
- If `col` or `row` is out of bounds, return.
- If `visited[idx]` is already set, return.
- If the cell is empty, return. In mode 0, if its color differs from `color`, return.
- Mark `visited[idx] = 1`, increment `popCount`.
- Recurse into the four neighbours (up, down, left, right).
- Preserve any register you need across the recursive call using `PUSH` / `POP`, and show the stack frame clearly in comments.
- The grid is only 208 cells, so recursion depth is safe.

`Match_Resolve(col, row)`:

1. Clear `visited` (using `REP STOSB`), set `popCount = 0`.
2. Run `Match_Fill` in mode 0 from the snapped cell.
3. If `popCount >= 3`, loop over the grid and set every visited cell to `EMPTY`, add score, and play a beep.
4. Run `Match_DropFloating`.

`Match_DropFloating`:

1. Clear `visited`, then run `Match_Fill` in mode 1 from **every non-empty cell in row 0**.
2. Every non-empty cell that is **not** visited is floating; set it to `EMPTY` and add `POINTS_DROP` per cell.

**Fallback:** if inline-asm recursion causes trouble that cannot be fixed simply, replace it with an explicit stack array and a loop (PUSH/POP-style), and document why in `docs/OUT_OF_OUTLINE.md`. Recursion is the preferred approach because it matches Week 13.

### 7.7 Ceiling drop (shows off string instructions)

`Grid_ShiftDown`: move every row down by one row. Use `STD` + `REP MOVSB` (or `MOVSD`) to copy memory from the end backward so it does not overwrite itself, then `CLD`. Then `Grid_AddTopRow` fills row 0 with random colors (calling `Con_Rand` for each cell). After shifting, run the danger-row check.

### 7.8 Number to text (Abdullah's module)

`Num_ToString(value, bufferAddress)`: convert an integer to decimal text by repeatedly dividing by 10 with `DIV`, pushing remainders on the stack, then popping them into the buffer (this reverses the order). C++ `Con_DrawText` then prints the buffer. Show this with comments, it demonstrates division and stack operations.

### 7.9 Main loop and state machine

`main()` (C++):

```
Con_Init();
Game_Reset();            // asm
while (gameState != STATE_EXIT) {
    Con_ReadKeys();      // C++: fills keyBits
    Game_Update();       // asm: whole state machine and logic for this frame
    Con_Sleep(30);       // C++: ~30 ms per frame
}
Con_Shutdown();
```

`Game_Update()` (asm): `CMP gameState` against each state constant and jump to the handler for menu / help / playing / paused / game-over. The playing handler does: read keys to change the aim; if Space and no shot active, fire; if a shot is active, run the movement sub-steps; redraw what changed.

### 7.10 Drawing without flicker (C++ side)

- Hide the console cursor.
- Draw the static frame (borders, labels, controls, danger line) **once**.
- After that, only redraw **what changed**: a grid cell when it changes, the moving bubble (erase old position, draw new one), the aim marker, and the HUD numbers when they change.
- Do not clear the whole screen every frame.
- For the grid, a simple approach is acceptable: redraw all cells only after a pop, drop, or ceiling shift (not every frame).

### 7.11 C++ service functions (called from assembly; names are suggestions, keep them simple)

```
void Con_Init();                              // hide cursor, set console size/title, clear screen
void Con_Shutdown();                          // restore cursor/colors
void Con_DrawFrame();                         // draw static border, labels, danger line, controls
void Con_DrawBubble(int col, int row, int color);  // color 0 = erase
void Con_DrawShot(int cellX, int cellY, int color); // draw/erase flying bubble
void Con_DrawAim(int index);                  // aim marker above the launcher
void Con_DrawLauncher(int curColor);          // [O] at the bottom
void Con_DrawText(int x, int y, const char* s, int colorAttr);
void Con_ReadKeys();                          // sets keyBits using GetAsyncKeyState
void Con_Sleep(int ms);
int  Con_Rand(int n);                         // returns 0..n-1
void Con_Beep(int freq, int ms);
void Con_ShowScreen(int which);               // menu / help / game-over text screens
```

---

## 8. Concept coverage (what the viva will look for)

| Project part | Assembly concept | Course week |
| --- | --- | --- |
| Grid as 2D array in 1D memory, `row*COLS+col` | 2D arrays, indirect/indexed addressing, `MUL` or shifts | 5, 7, 14 |
| Scanning grid, counting, loops | Arrays and loops, `LOOP`, `CMP`, jumps | 7, 9 |
| State machine, key bit tests | Conditional processing, `TEST`, `AND`, `OR`, flags | 4, 9, 10 |
| Aim table and lookup | Arrays, indirect addressing, data definitions | 2, 3, 5 |
| Fixed-point shot movement | Shifts (`SAR`, `SHL`), `IMUL`, `NEG`, signed jumps | 12 |
| Flood-fill | Recursion, stack frames, `PUSH`/`POP`/`CALL`/`RET` | 9, 13 |
| Clearing `visited`, shifting rows | String instructions `REP STOSB`, `REP MOVSB`, direction flag | 14 |
| Score, number-to-text | `DIV`, `MUL`, stack, remainders | 12 |
| Mixing C++ and assembly | Inline assembly, calling conventions | 15 |

---

## 9. Feature list and priority

**Must have (build in this order):**

1. Project skeleton, `build.bat`, console wrapper, static frame drawn on screen
2. Grid data, random fill, draw grid
3. Aim with arrow keys, launcher and aim marker drawn
4. Fire, shot movement, wall bounce, snapping
5. Flood-fill match and pop, floating-bubble drop
6. Score, level, high score, HUD with number-to-text
7. Next-bubble preview, ceiling drop with "DROP IN" counter
8. Game over, pause, menu, help screen, restart
9. Level progression when the grid is cleared

**Optional extras (only after everything above works and is tested):**

- Beep sound effects (pop, drop, game over)
- High score saved to a text file (C++ file I/O; document it in `OUT_OF_OUTLINE.md`)

Do **not** add power-ups, hex grids, mouse control, multiple threads, or graphics.

---

## 10. Build and run

Provide `build.bat` for the **"x86 Native Tools Command Prompt for VS 2022"**:

```
cl /nologo /W3 /EHsc /Fe:BubbleShooter.exe src\*.cpp
```

Use `#pragma warning(disable:4035)` where a function leaves its result in `EAX` from an `__asm` block. Optionally also provide a Visual Studio project (`.vcxproj`) set to **Debug | Win32**. Tell the user exactly how to build and run in `README.md`.

**If you cannot compile or run Windows/MSVC code in your own environment:** do not pretend you tested it. Write the code carefully, re-read each `__asm` block for the pitfalls below, and ask the user to build, run, and paste back compiler errors or behaviour. Fix and repeat.

### Inline-assembly pitfalls checklist (MSVC x86)

- You don't need to preserve `EAX, EBX, ECX, EDX, ESI, EDI` inside an `__asm` block, but you **must not break `ESP` and `EBP`**.
- When calling a C++ function from `__asm`: push arguments **right to left**, `call Name`, then `add esp, 4*numArgs`. The callee may change `EAX, ECX, EDX`, so save what you need with `PUSH`/`POP`.
- Global variables and arrays can be used by **name** in `__asm` (for example `mov eax, score`). Use `lea esi, grid` to get an address. Function parameters can also be used by name.
- Labels inside one function must be **unique**, and must not clash with C++ names.
- Always `CLD` after using `STD`.
- Do not mix `__asm` with `__declspec(naked)` unless you fully understand the stack frame. Prefer normal functions.
- Compile a **tiny test first** (one function that adds two numbers in `__asm`) before building on top of it.

---

## 11. Proving the 70 / 30 ratio

Create `tools/ratio.ps1` (PowerShell, works on any Windows) that:

1. Scans all `src\*.cpp` and `src\*.h` files.
2. Counts **non-blank, non-comment lines**.
3. Counts lines **inside `__asm { ... }` blocks** (and one-line `__asm` statements) as **assembly**; all other counted lines are **C++**.
4. Prints assembly lines, C++ lines, and the percentage of each, per file and in total.
5. Saves the result to `docs/RATIO.md`.

Run it after every major step. If assembly drops below **72%**, move more logic into assembly (never inflate the number with pointless instructions). Keep C++ short: no unnecessary helper functions, no long comments counted as code, and no large data tables in C++ beyond what is needed.

---

## 12. Documents you must produce (for the report and the viva)

1. **`README.md`** — what the game is, how to build and run, controls, project structure, how inline assembly is used, the 70/30 split explanation, the `PROC` vs inline-function mapping note (section 3), and a short statement that Irvine32 is not linked because it is built for stand-alone MASM projects and is awkward to mix with inline assembly (the Win32 console functions replace `Gotoxy`, `SetTextColor`, `ReadKey`, `Delay`).
2. **`docs/CODE_WALKTHROUGH.md`** — for every source file and every function: purpose, inputs and outputs, registers used, and a plain-English explanation. Written so a member can answer "explain this function" in the viva.
3. **`docs/OUT_OF_OUTLINE.md`** — see section 4.
4. **`docs/AI_DISCLOSURE.md`** — the course requires AI-generated code to be disclosed. State that the code was generated with the help of Codex, list which files and functions were AI-generated, and what the team reviewed and changed by hand (leave a table the team can fill in).
5. **`docs/TESTING.md`** — checklist from section 13, with pass/fail columns the user can fill in.

---

## 13. Acceptance checklist

- [ ] Builds with no errors using `build.bat` (x86)
- [ ] Menu appears; `1` starts, `2` shows help, `3` exits
- [ ] Grid shows colored bubbles; layout matches the mock-up
- [ ] Left/Right changes the aim marker across 9 angles; it stops at both ends
- [ ] Space fires; the bubble travels in the aimed direction and bounces off both walls
- [ ] The bubble snaps into the correct empty cell, including at the ceiling
- [ ] 3+ same-colored connected bubbles pop; 2 do not
- [ ] Bubbles disconnected from the top fall and score extra
- [ ] Score, level and high score update correctly and display as numbers
- [ ] Next bubble preview is correct, and the current bubble takes its color after a shot
- [ ] "DROP IN" counts down, the ceiling shifts down, a new top row appears, and the counter resets after a pop
- [ ] A bubble reaching the danger row ends the game; the game-over screen shows the final score
- [ ] P pauses and resumes; ESC returns to the menu; a new game after game-over works with a clean state
- [ ] Clearing the grid moves to the next level with more rows
- [ ] No flicker during normal play; no crash after long play
- [ ] `docs/RATIO.md` shows **≥ 72% assembly**
- [ ] Every `__asm` block is commented; all docs in section 12 exist

---

## 14. How you should work (process rules for Codex)

1. **Work in the milestone order of section 9.** One milestone at a time. Finish, make sure it builds, then move on.
2. **At the end of each milestone, reply with:** what you built, which files changed, how to build and test it, what the user should see, and the current assembly/C++ ratio.
3. **Explain simply.** The user is a student who must defend this project in a viva. Prefer clear, boring code over clever code.
4. **Don't silently change the design.** If a rule in this file cannot be followed, stop and explain the problem and the options before changing anything.
5. **Don't add features** that are not in this file.
6. **Don't refactor working code** without a reason; if you must, say why.
7. **Keep the file ownership** from section 6.2 so each member can explain their part.
8. **Never claim you ran or tested something you didn't.**
9. When something is outside the course outline, follow section 4.
10. Write all explanations and comments in **simple English**.

---

## 15. First task

Start with **Milestone 1**:

1. Create the folder structure from section 6.1.
2. Write `build.bat` and a tiny proof-of-concept: one C++ function with an `__asm` block that adds two numbers, and `main` that prints the result, so we know inline assembly compiles on the user's machine.
3. Write `console.cpp/.h` and `game.h`, and `data.cpp` with the constants and globals.
4. Draw the static game frame and the side-panel labels exactly as in the mock-up.
5. Create `tools/ratio.ps1` and `README.md` (initial version).
6. Tell the user how to build and run, then wait for confirmation before Milestone 2.
