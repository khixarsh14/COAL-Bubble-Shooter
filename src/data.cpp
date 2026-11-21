// Owner: shared data; Usman Khan coordinates integration with all three members.
// Concepts: zero-initialized arrays, integers, and precomputed direction tables.
#include "game.h"

BYTE grid[COLS * ROWS] = {0};
BYTE visited[COLS * ROWS] = {0};
// Fixed-point data only; no movement yet. See docs/OUT_OF_OUTLINE.md, Fixed-point data.
int angleDX[NUM_ANGLES] = {-222, -181, -128, -66, 0, 66, 128, 181, 222};
int angleDY[NUM_ANGLES] = {128, 181, 222, 247, 256, 247, 222, 181, 128};
int aimIndex = 4;
int curColor = 1, nextColor = 1;
int shotActive = 0;
int shotX = 0, shotY = 0, shotDX = 0, shotDY = 0;
int lastCol = 0, lastRow = 0;
int score = 0, highScore = 0;
int shotsUntilDrop = DROP_INTERVAL;
int gameState = STATE_MENU;
int keyBits = 0, popCount = 0;
char numBuf[12] = {0};
