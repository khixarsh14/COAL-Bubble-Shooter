// Owner: Khizar Sheikh.
// Concepts: isolated assertions, array snapshots, and deterministic test seeding.
// Test code stays outside src/ and does not enter the normal preview executable.
#include "../src/game.h"
#include "../src/console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static int checks = 0, failures = 0;

static void Check(int condition, const char* description)
{
    ++checks;
    if (!condition) ++failures;
    printf("%s: %s\n", condition ? "PASS" : "FAIL", description);
}

static void TestInit()
{
    BYTE first[COLS * ROWS];
    memset(grid, 255, sizeof(grid));
    memset(visited, 123, sizeof(visited));
    srand(1234);
    Grid_Init();
    bool valid = STARTING_ROWS == 5;
    for (int i = 0; i < COLS * ROWS; ++i) {
        if (i < STARTING_ROWS * COLS) {
            if (grid[i] < 1 || grid[i] > NUM_COLORS) valid = false;
        } else if (grid[i] != EMPTY) valid = false;
        if (visited[i] != 123) valid = false;
    }
    Check(valid, "Init fills five rows and clears the rest without touching visited");
    memcpy(first, grid, sizeof(grid));
    srand(1234);
    Grid_Init();
    Check(memcmp(first, grid, sizeof(grid)) == 0, "Init repeatable with same CRT seed");
}

static void TestGetSet()
{
    memset(grid, 0, sizeof(grid));
    int valid = 1;
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int color = (row * COLS + col) % (NUM_COLORS + 1);
            BYTE before[COLS * ROWS];
            memcpy(before, grid, sizeof(grid));
            Grid_Set(col, row, color);
            for (int i = 0; i < COLS * ROWS; ++i) {
                int expected = i == row * COLS + col ? color : before[i];
                if (grid[i] != expected) valid = 0;
            }
            if (Grid_Get(col, row) != color) valid = 0;
        }
    }
    Check(valid, "Get/Set every cell, including EMPTY and all five colors; no neighbor writes");
    const int bad[] = {INT_MIN, -100, -1, COLS, ROWS, 100, INT_MAX};
    BYTE before[COLS * ROWS];
    memcpy(before, grid, sizeof(grid));
    valid = 1;
    for (int i = 0; i < 7; ++i) {
        if (bad[i] < 0 || bad[i] >= COLS) {
            if (Grid_Get(bad[i], 0) != EMPTY) valid = 0;
            Grid_Set(bad[i], 0, 1);
        }
        if (bad[i] < 0 || bad[i] >= ROWS) {
            if (Grid_Get(0, bad[i]) != EMPTY) valid = 0;
            Grid_Set(0, bad[i], 1);
        }
    }
    Grid_Set(-1, -1, 1);
    Check(valid && !memcmp(before, grid, sizeof(grid)), "Invalid coordinates return EMPTY and leave grid unchanged");
    const int badColors[] = {INT_MIN, -1, NUM_COLORS + 1, 256, INT_MAX};
    for (int i = 0; i < 5; ++i) Grid_Set(7, 6, badColors[i]);
    Check(!memcmp(before, grid, sizeof(grid)), "Invalid colors rejected before truncating to BYTE");
    Grid_Set(COLS - 1, ROWS - 1, EMPTY);
    Check(Grid_Get(COLS - 1, ROWS - 1) == EMPTY, "Set EMPTY erases final cell");
}

static void TestShiftAndTop()
{
    BYTE before[COLS * ROWS];
    // Distinct nonzero bytes detect every source/destination offset, not just colors.
    for (int i = 0; i < COLS * ROWS; ++i) grid[i] = (BYTE)(i + 1);
    memcpy(before, grid, sizeof(grid));
    Grid_ShiftDown();
    int valid = 1;
    for (int i = 0; i < COLS; ++i) if (grid[i] != EMPTY) valid = 0;
    Check(valid && !memcmp(grid + COLS, before, sizeof(grid) - COLS),
        "ShiftDown copies all 192 bytes backward, discards old last row, clears top");
    memcpy(before, grid, sizeof(grid));
    srand(42);
    Grid_AddTopRow();
    valid = 1;
    for (int i = 0; i < COLS; ++i) if (grid[i] < 1 || grid[i] > NUM_COLORS) valid = 0;
    Check(valid && !memcmp(grid + COLS, before + COLS, sizeof(grid) - COLS),
        "AddTopRow fills 16 colors and preserves all lower rows");
    for (int i = 0; i < ROWS; ++i) Grid_ShiftDown();
    Check(Grid_IsEmpty(), "Repeated ShiftDown eventually empties grid (forward stores work after STD)");
}

static void TestScans()
{
    memset(grid, 0, sizeof(grid));
    Check(Grid_IsEmpty() == 1 && Grid_CheckDanger() == 0, "Empty grid returns exact boolean results");
    int valid = 1;
    for (int i = 0; i < COLS * ROWS; ++i) {
        grid[i] = (BYTE)(i % NUM_COLORS + 1);
        if (Grid_IsEmpty() != 0 || Grid_CheckDanger() != (i >= DANGER_ROW * COLS)) valid = 0;
        grid[i] = EMPTY;
    }
    Check(valid, "Each of 208 individual positions detected; danger only in rows 11 and 12");
    grid[COLS * ROWS - 1] = 255;
    Check(Grid_IsEmpty() == 0 && Grid_CheckDanger() == 1, "Scans treat any nonzero last byte as occupied");
    memset(grid, 0, sizeof(grid));
}

static void TestRandom()
{
    int valid = Con_Rand(0) == 0 && Con_Rand(-1) == 0;
    for (int i = 0; i < 1000; ++i) {
        int value = Con_Rand(NUM_COLORS);
        if (value < 0 || value >= NUM_COLORS) valid = 0;
    }
    Check(valid, "Real Con_Rand service returns bounded values and handles n <= 0");
}

int main()
{
    TestInit();
    TestGetSet();
    TestShiftAndTop();
    TestScans();
    TestRandom();
    printf("Grid tests: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
