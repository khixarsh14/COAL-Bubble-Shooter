#include "../src/game.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int checks, failures;

static void Check(bool okay, const char* name)
{
    ++checks;
    if (!okay) ++failures;
    printf("%s: %s\n", okay ? "PASS" : "FAIL", name);
}

static void Tick(int keys)
{
    keyBits = keys;
    Game_Update();
}

static void ShotAt(int col, int y, int dx, int dy)
{
    shotActive = 1;
    shotX = col * SUBPIX + SUBPIX / 2;
    shotY = y;
    shotDX = dx;
    shotDY = dy;
    lastCol = lastRow = -1;
    curColor = 2;
    nextColor = 4;
}

static void TestShooter()
{
    Game_Reset();
    memset(grid, 0, sizeof(grid));
    Shooter_Aim(INT_MAX);
    Check(aimIndex == NUM_ANGLES - 1, "Aim clamps right, including positive overflow");
    Shooter_Aim(INT_MIN);
    Check(aimIndex == 0, "Aim clamps left");
    Shooter_Fire();
    Check(shotActive && shotDX == angleDX[0] && shotDY == -angleDY[0], "Fire loads signed direction");
    const int oldX = shotX;
    Shooter_Aim(4);
    Shooter_Fire();
    Check(oldX == shotX && shotDX == angleDX[0], "Fire leaves an active shot alone");

    ShotAt(0, 10 * SUBPIX, -222, -128);
    shotX = 1;
    Shooter_Update();
    Check(shotActive && shotDX == 222 && shotX > 0, "Left wall reflects into board");
    ShotAt(COLS - 1, 10 * SUBPIX, 222, -128);
    shotX = COLS * SUBPIX - 2;
    Shooter_Update();
    Check(shotActive && shotDX == -222 && shotX < COLS * SUBPIX, "Right wall reflects into board");

    ShotAt(4, SUBPIX + 1, 0, -SUBPIX);
    Shooter_Update();
    Check(!shotActive && Grid_Get(4, 0) == 2 && curColor == 4 &&
          nextColor >= 1 && nextColor <= NUM_COLORS, "Open ceiling snaps and promotes next color");
    ShotAt(4, 2 * SUBPIX + 2, 0, -SUBPIX - 16);
    Shooter_Update();
    Shooter_Update();
    Check(!shotActive && Grid_Get(4, 0) == 2 && Grid_Get(4, 1) == 2,
          "Occupied ceiling preserves the struck cell and uses the last empty cell");

    memset(grid, 0, sizeof(grid));
    Grid_Set(4, ROWS - 1, 3);
    ShotAt(4, ROWS * SUBPIX + 1, 0, -SUBPIX);
    Shooter_Update();
    Check(!shotActive && lastRow == -1 && Grid_Get(4, ROWS - 1) == 3 && curColor == 2,
          "Blocked launch leaves the grid and color queue intact");

    memset(grid, 0, sizeof(grid));
    Grid_Set(4, 2, 3);
    ShotAt(3, 2 * SUBPIX + 40, 222, -SUBPIX);
    shotX = 4 * SUBPIX - 16;
    lastCol = 3;
    lastRow = 2;
    Shooter_Update();
    Check(!shotActive && Grid_Get(3, 2) == 2 && Grid_Get(4, 2) == 3,
          "A diagonal substep hits the horizontal intermediate cell first");

    memset(grid, 0, sizeof(grid));
    Grid_Set(3, 1, 3);
    ShotAt(3, 2 * SUBPIX + 10, 222, -SUBPIX);
    shotX = 4 * SUBPIX - 44;
    lastCol = 3;
    lastRow = 2;
    Shooter_Update();
    Check(!shotActive && Grid_Get(3, 2) == 2 && Grid_Get(3, 1) == 3,
          "A diagonal substep hits the vertical intermediate cell first");
}

static void TestStates()
{
    gameState = STATE_MENU;
    Tick(0);
    Tick(KEY_2 | KEY_ANY);
    Check(gameState == STATE_HELP, "Menu 2 shows help");
    Tick(KEY_ESC | KEY_ANY);
    Check(gameState == STATE_MENU, "Help ESC goes back");
    Tick(KEY_ESC);
    Check(gameState == STATE_MENU, "Held ESC cannot exit on the next state");
    Tick(KEY_1 | KEY_ANY);
    Check(gameState == STATE_PLAYING && level == 1 && shotsUntilDrop == 7, "Menu 1 starts a clean round");
    Tick(KEY_SPACE | KEY_ANY);
    const int y = shotY;
    Tick(KEY_P | KEY_ANY);
    Check(gameState == STATE_PAUSED && shotY == y, "Pause freezes a shot before movement");
    Tick(KEY_P);
    Check(gameState == STATE_PAUSED && shotY == y, "Held P does not resume");
    Tick(0);
    Tick(KEY_P | KEY_ANY);
    Check(gameState == STATE_PLAYING && shotY == y, "Fresh P resumes without stepping this frame");

    memset(grid, 0, sizeof(grid));
    Grid_Set(0, 0, 2);
    Grid_Set(1, 0, 2);
    score = 0;
    ShotAt(2, SUBPIX + 1, 0, -SUBPIX);
    Tick(0);
    Check(score == 30 && level == 2 && shotsUntilDrop == 6 &&
          Grid_Get(0, 5) != EMPTY && Grid_Get(0, 6) == EMPTY,
          "One placement scores once and refills an empty board for the next level");

    memset(grid, 0, sizeof(grid));
    Grid_Set(8, DANGER_ROW - 1, 1);
    shotsUntilDrop = 1;
    highScore = 0;
    ShotAt(2, SUBPIX + 1, 0, -SUBPIX);
    Tick(0);
    Check(gameState == STATE_GAMEOVER && Grid_Get(8, DANGER_ROW) == 1 &&
          Grid_Get(2, 1) == 2 && highScore == 30, "Last miss shifts exactly once and detects danger");
    Tick(KEY_ANY);
    Check(gameState == STATE_MENU, "A fresh unrelated key returns from game over");
    Tick(KEY_1);
    Check(gameState == STATE_PLAYING && score == 0 && highScore == 30, "Restart retains the session high score");

    memset(grid, 0, sizeof(grid));
    Grid_Set(4, DANGER_ROW - 1, 3);
    ShotAt(4, (DANGER_ROW + 1) * SUBPIX - 1, 0, -SUBPIX);
    for (int frame = 0; shotActive && frame < ROWS + 1; ++frame) Tick(0);
    Check(gameState == STATE_GAMEOVER && Grid_Get(4, DANGER_ROW) == 2,
          "A surviving placement in the danger row ends the round");
    Tick(KEY_ANY);
    Tick(KEY_3);
    Check(gameState == STATE_EXIT, "Menu 3 exits");
}

static void TestScore()
{
    Num_ToString(0, numBuf);
    Check(strcmp(numBuf, "0") == 0, "Format zero");
    Num_ToString(INT_MIN, numBuf);
    Check(strcmp(numBuf, "-2147483648") == 0, "Format INT_MIN");
    Num_ToString(INT_MAX, numBuf);
    Check(strcmp(numBuf, "2147483647") == 0, "Format INT_MAX");
    score = 4;
    Score_AddPop(3);
    Score_AddDrop(2);
    Score_AddPop(-1);
    Check(score == 74, "Standalone point helpers apply the correct multipliers");
    level = 99;
    Score_UpdateDrop(1);
    Score_UpdateDrop(0);
    Check(shotsUntilDrop == 2, "High-level drop interval is clamped to three");
}

int main()
{
    TestShooter();
    TestStates();
    TestScore();
    printf("Game tests: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
