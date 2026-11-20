// Owner: shared interface; Usman Khan coordinates integration.
// Concepts: constants, external data, and cdecl function declarations.
#ifndef BUBBLE_GAME_H
#define BUBBLE_GAME_H

#if !defined(_MSC_VER) || defined(__clang__) || !defined(_M_IX86)
#error Build with Microsoft MSVC targeting x86 (Win32).
#endif
#include <windows.h>

enum {
    COLS = 16, ROWS = 13, DANGER_ROW = 11,
    EMPTY = 0, NUM_COLORS = 5, NUM_ANGLES = 9,
    MIN_MATCH = 3, POINTS_POP = 10, POINTS_DROP = 20, SUBPIX = 256
};
enum {
    STATE_MENU = 0, STATE_PLAYING = 1, STATE_PAUSED = 2,
    STATE_GAMEOVER = 3, STATE_HELP = 4, STATE_EXIT = 5
};
enum {
    KEY_LEFT = 1, KEY_RIGHT = 2, KEY_SPACE = 4, KEY_P = 8,
    KEY_ESC = 16, KEY_1 = 32, KEY_2 = 64, KEY_3 = 128, KEY_ANY = 256
};

extern BYTE grid[COLS * ROWS], visited[COLS * ROWS];
extern int angleDX[NUM_ANGLES], angleDY[NUM_ANGLES];
extern int aimIndex, curColor, nextColor, shotActive;
extern int shotX, shotY, shotDX, shotDY, lastCol, lastRow;
extern int score, level, highScore, shotsUntilDrop;
extern int gameState, keyBits, popCount;
extern char numBuf[12];

// Reserved interfaces for later milestones: declarations only, never called yet.
// Usman Khan
void __cdecl Game_Reset();
void __cdecl Game_Update();
void __cdecl Shooter_Aim(int step);
void __cdecl Shooter_Fire();
void __cdecl Shooter_Update();
// Khizar Sheikh
void __cdecl Grid_Init();
int __cdecl Grid_Get(int col, int row);
void __cdecl Grid_Set(int col, int row, int color);
void __cdecl Grid_ShiftDown();
void __cdecl Grid_AddTopRow();
int __cdecl Grid_CheckDanger();
int __cdecl Grid_IsEmpty();
void __cdecl Match_Fill(int col, int row, int color, int mode);
void __cdecl Match_Resolve(int col, int row);
void __cdecl Match_DropFloating();
// Abdullah bin Faheem
void __cdecl Score_AddPop(int count);
void __cdecl Score_AddDrop(int count);
void __cdecl Score_UpdateLevel();
void __cdecl Score_UpdateDrop(int popped);
void __cdecl Num_ToString(int value, char* bufferAddress);

#endif
