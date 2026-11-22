# COAL Bubble Shooter

A Windows console Bubble Shooter for FAST-NUCES COAL Lab. Game logic uses MSVC x86 inline assembly. C++ provides startup and Win32 services.

## Build and play

Install Visual Studio 2026 with Desktop development with C++, the v145 MSVC x86 tools, and a Windows SDK. Open the **x86 Native Tools Command Prompt for VS 2026**, change to this repository, and run:

```bat
build.bat
bin\BubbleShooter.exe
```

The build runs the separate assembly proof and then builds the game. For Visual Studio, use **Debug | Win32** in `BubbleShooter.sln` and select **BubbleShooter** as the startup project.

- Main menu. **1** starts, **2** shows help, **3** exits.
- Round. **Left/Right** aim, **Space** shoots, **P** pauses or resumes, and **Esc** returns to the menu. On game over, press any key to return to the menu.
- The dotted guide shows the next shot's path, including wall bounces. The `+` marks its predicted landing cell. The guide returns after each shot.
- Connect three same-color bubbles orthogonally to pop them. Detached bubbles fall for bonus points. Seven consecutive misses lower the ceiling, and a bubble surviving on or below the danger line ends the round. Clearing the board wins the round. A win or loss returns to the menu on a fresh keypress. The high score lasts for the current session.

## Verify

From the same x86 prompt:

```bat
build.bat
tests\build_game_tests.bat
tests\build_match_tests.bat
tests\build_grid_tests.bat
powershell -NoProfile -ExecutionPolicy Bypass -File tools\ratio.ps1
```

## Source map

- `src/shooter.cpp` handles aiming, fixed-point projectile motion, and placement.
- `src/game.cpp` owns round state, input edges, and the order of post-shot updates.
- `src/grid.cpp` implements grid storage operations. `src/match.cpp` owns flood-fill, removal, and points awarded for a placement.
- `src/score.cpp` owns the miss counter, ceiling shifts and decimal conversion.
- `src/console.cpp` polls Win32 input, composes the board and HUD, and draws changed cells. `src/main.cpp` owns the console lifetime and frame loop.
- `src/game.h` and `src/console.h` declare shared cdecl interfaces. `src/data.cpp` defines shared storage.

A C++ function declaration and definition play the roles of MASM `PROTO` and `PROC`. A normal C++ call or a cdecl push/call sequence supplies the role of `INVOKE`, and an address parameter supplies the role of `ADDR`. Win32 console services provide cursor positioning, text attributes, input, and timing. The project does not link Irvine32.
