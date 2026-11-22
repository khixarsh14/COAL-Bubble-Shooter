#include "../src/game.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
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

static void TestPrediction()
{
    const int knownLanding[] = {14, LAUNCH_COL, 2};
    const int sampleAngles[] = {0, 4, 8};
    BYTE guide[COLS * ROWS];
    memset(grid, 0, sizeof(grid));
    for (int example = 0; example < 3; ++example) {
        Shooter_Predict(sampleAngles[example], guide);
        bool touchedWall = false;
        const int wall = example == 0 ? 0 : COLS - 1;
        for (int row = 1; row < ROWS; ++row)
            touchedWall |= guide[row * COLS + wall] == GUIDE_DOT;
        Check(guide[knownLanding[example]] == GUIDE_LANDING && (example == 1 || touchedWall),
              example == 0 ? "Guide reflects at the left wall before landing" :
              example == 2 ? "Guide reflects at the right wall before landing" :
              "Straight guide reaches the open ceiling at the launch column");
    }

    bool matchesLiveShot = true;
    for (int fixture = 0; fixture < 3; ++fixture) {
        for (int index = 0; index < NUM_ANGLES; ++index) {
            memset(grid, 0, sizeof(grid));
            if (fixture == 1) {
                for (int col = 0; col < COLS; ++col) Grid_Set(col, col % 3, 3);
                Grid_Set(1, 8, 3);
                Grid_Set(14, 7, 3);
            }
            if (fixture == 2) Grid_Set(LAUNCH_COL, ROWS - 1, 3);
            Shooter_Predict(index, guide);
            int target = -1, targets = 0;
            for (int cell = 0; cell < COLS * ROWS; ++cell) {
                if (guide[cell] != 0 && grid[cell] != EMPTY) matchesLiveShot = false;
                if (guide[cell] != GUIDE_LANDING) continue;
                target = cell;
                ++targets;
            }
            shotActive = 0;
            aimIndex = index;
            curColor = 2;
            nextColor = 4;
            Shooter_Fire();
            for (int frame = 0; frame < 32 && shotActive; ++frame) Shooter_Update();
            const int actual = lastRow < 0 ? -1 : lastRow * COLS + lastCol;
            matchesLiveShot &= !shotActive && target == actual && targets == (actual >= 0);
            if (actual >= 0) matchesLiveShot &= Grid_Get(lastCol, lastRow) == 2;
        }
    }
    Check(matchesLiveShot, "All nine guides agree with live shots on three fixtures including blocked launches");

    memset(grid, 0, sizeof(grid));
    Grid_Set(LAUNCH_COL, 5, 3);
    Shooter_Predict(4, guide);
    Check(guide[6 * COLS + LAUNCH_COL] == GUIDE_LANDING &&
          guide[5 * COLS + LAUNCH_COL] == 0 && guide[4 * COLS + LAUNCH_COL] == 0,
          "Guide stops at the last empty cell before an obstacle");

    BYTE originalGrid[sizeof(grid)], originalVisited[sizeof(visited)];
    memcpy(originalGrid, grid, sizeof(grid));
    memset(visited, 123, sizeof(visited));
    memcpy(originalVisited, visited, sizeof(visited));
    ShotAt(5, 2500, 128, -222);
    const int* const state[] = {&shotX, &shotY, &shotDX, &shotDY, &lastCol, &lastRow,
        &shotActive, &aimIndex, &curColor, &nextColor, &score, &popCount,
        &shotsUntilDrop, &gameState, &keyBits, &highScore};
    int originalState[_countof(state)];
    for (unsigned i = 0; i < _countof(state); ++i) originalState[i] = *state[i];
    srand(1234);
    const int expectedRandom = rand();
    srand(1234);
    for (int index = 0; index < NUM_ANGLES; ++index) Shooter_Predict(index, guide);
    bool unchanged = memcmp(originalGrid, grid, sizeof(grid)) == 0 &&
        memcmp(originalVisited, visited, sizeof(visited)) == 0 && rand() == expectedRandom;
    for (unsigned i = 0; i < _countof(state); ++i) unchanged &= originalState[i] == *state[i];
    Check(unchanged, "Prediction preserves board, match scratch, live shot, game state and RNG");
}

static void TestAimRepeat()
{
    Game_Reset();
    const int center = NUM_ANGLES / 2;
    Game_UpdateAt(1000);
    keyBits = KEY_LEFT;
    Game_UpdateAt(1010);
    Game_UpdateAt(1030);
    keyBits = 0;
    Game_UpdateAt(1050);
    Check(aimIndex == center - 1, "A short arrow tap changes aim exactly once");
    keyBits = KEY_RIGHT;
    Game_UpdateAt(1100);
    Game_UpdateAt(1219);
    Check(aimIndex == center, "Held aim steps immediately and waits for the repeat interval");
    Game_UpdateAt(1220);
    Game_UpdateAt(1339);
    Check(aimIndex == center + 1, "Aim repeats at 120 ms independently of frame count");
    Game_UpdateAt(1800);
    Check(aimIndex == center + 2, "A delayed frame produces only one repeat");
    keyBits = KEY_LEFT;
    Game_UpdateAt(1801);
    Check(aimIndex == center + 1, "Changing direction steps immediately");
    keyBits = KEY_LEFT | KEY_RIGHT;
    Game_UpdateAt(1900);
    Check(aimIndex == center + 1, "Opposite arrows cancel aim");
    keyBits = KEY_LEFT;
    Game_UpdateAt(0xFFFFFFF0u);
    Game_UpdateAt(0x67u);
    Check(aimIndex == center, "Tick rollover preserves the wait before the repeat");
    Game_UpdateAt(0x68u);
    Check(aimIndex == center - 1, "Aim repeats on time after tick rollover");
    keyBits = 0;
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
    Check(gameState == STATE_PLAYING && shotsUntilDrop == 7, "Menu 1 starts a clean round with seven misses allowed");
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
    highScore = 0;
    shotsUntilDrop = 1;
    ShotAt(2, SUBPIX + 1, 0, -SUBPIX);
    Tick(0);
    Check(gameState == STATE_WIN && Grid_IsEmpty() && score == 30 && highScore == 30 &&
          shotsUntilDrop == 7, "Clearing the board wins, scores once, and resets the drop counter");
    Tick(0);
    Check(gameState == STATE_WIN && Grid_IsEmpty(), "Victory leaves the board empty until restart");
    Tick(KEY_ANY);
    Check(gameState == STATE_MENU, "A fresh key returns from victory");
    Tick(KEY_1);
    Check(gameState == STATE_PLAYING && score == 0 && highScore == 30 && shotsUntilDrop == 7 &&
          Grid_Get(0, 4) != EMPTY && Grid_Get(0, 5) == EMPTY, "Restart after victory creates a fresh five-row board");

    memset(grid, 0, sizeof(grid));
    Grid_Set(15, DANGER_ROW - 1, 1);
    bool countdown = true;
    for (int miss = 0; miss < 6; ++miss) {
        ShotAt(miss, SUBPIX + 1, 0, -SUBPIX);
        curColor = 2 + miss % 2;
        Tick(0);
        countdown &= gameState == STATE_PLAYING && shotsUntilDrop == 6 - miss &&
            Grid_Get(15, DANGER_ROW - 1) == 1 && Grid_Get(15, DANGER_ROW) == EMPTY;
    }
    Check(countdown, "The first six misses do not shift the ceiling");
    ShotAt(6, SUBPIX + 1, 0, -SUBPIX);
    Tick(0);
    Check(gameState == STATE_GAMEOVER && Grid_Get(15, DANGER_ROW) == 1 &&
          Grid_Get(6, 1) == 2 && highScore == 30 && shotsUntilDrop == 7,
          "The seventh miss shifts once, resets the counter, and detects danger");
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
    shotsUntilDrop = 2;
    Score_UpdateDrop(1);
    Score_UpdateDrop(0);
    Check(shotsUntilDrop == 6, "A pop restores seven misses and the next miss consumes one");
}

int main()
{
    TestShooter();
    TestPrediction();
    TestAimRepeat();
    TestStates();
    TestScore();
    printf("Game tests: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
