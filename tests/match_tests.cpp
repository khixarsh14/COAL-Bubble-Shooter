// Owner: Khizar Sheikh.
// Concepts: deterministic grids, recursive-fill checks, and x86 ESP inspection.
#include "../src/game.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>

static int checks = 0, failures = 0;

static void Check(int condition, const char* description)
{
    ++checks;
    if (!condition) ++failures;
    printf("%s: %s\n", condition ? "PASS" : "FAIL", description);
}

static void Reset()
{
    memset(grid, 0, sizeof(grid));
    memset(visited, 0, sizeof(visited));
    popCount = 0;
    score = 0;
    highScore = 0;
    shotsUntilDrop = 7;
}

static int VisitedSum()
{
    int sum = 0;
    for (int i = 0; i < COLS * ROWS; ++i) sum += visited[i];
    return sum;
}

static int Occupied()
{
    int count = 0;
    for (int i = 0; i < COLS * ROWS; ++i) if (grid[i]) ++count;
    return count;
}

static void TestFill()
{
    Reset();
    grid[0] = grid[1] = grid[COLS] = 2;
    grid[2] = 3;
    grid[2 * COLS + 1] = 2; // Diagonal from (0,1), isolated in mode 0.
    Match_Fill(0, 0, 2, 0);
    Check(popCount == 3 && VisitedSum() == 3 && !visited[2] && !visited[2 * COLS + 1],
        "Fill: exact color and four neighbours; different color and diagonal excluded");
    Check(Occupied() == 5 && score == 0, "Fill: grid and score unchanged");
    Match_Fill(0, 0, 2, 0);
    Check(popCount == 3 && VisitedSum() == 3, "Fill: already visited start is not counted again");
    memset(visited, 0, sizeof(visited));
    popCount = 0;
    Match_Fill(0, 0, 999, 1);
    Check(popCount == 4 && VisitedSum() == 4, "Fill: mode 1 connects different colors and ignores color argument");

    Reset();
    grid[0] = grid[COLS - 1] = grid[(ROWS - 1) * COLS] = grid[COLS * ROWS - 1] = 1;
    Match_Fill(0, 0, 1, 0);
    Match_Fill(COLS - 1, 0, 1, 0);
    Match_Fill(0, ROWS - 1, 1, 0);
    Match_Fill(COLS - 1, ROWS - 1, 1, 0);
    Check(popCount == 4 && VisitedSum() == 4, "Fill: all four corners terminate safely");
    BYTE before[COLS * ROWS], marks[COLS * ROWS];
    memcpy(before, grid, sizeof(grid));
    memcpy(marks, visited, sizeof(visited));
    const int invalid[][2] = {{-1,0},{0,-1},{COLS,0},{0,ROWS},{INT_MIN,INT_MAX},{INT_MAX,INT_MIN}};
    for (int i = 0; i < 6; ++i) Match_Fill(invalid[i][0], invalid[i][1], 1, 0);
    Match_Fill(1, 1, 1, 0); // Empty start.
    Check(popCount == 4 && !memcmp(before, grid, sizeof(grid)) && !memcmp(marks, visited, sizeof(visited)),
        "Fill: invalid and empty starts preserve arrays and count");
    Reset();
    grid[0] = 1;
    Match_Fill(0, 0, 257, 0);
    Match_Fill(0, 0, 1, 2);
    Check(popCount == 0 && VisitedSum() == 0, "Fill: full-width mismatched color and invalid mode rejected");

    Reset();
    memset(grid, 4, sizeof(grid));
    unsigned int beforeEsp = 0, afterEsp = 0;
    // Read ESP around an ordinary top-level C++ cdecl call, not a custom wrapper.
    __asm {
        mov beforeEsp, esp
    }
    Match_Fill(0, 0, 4, 0);
    __asm {
        mov afterEsp, esp
    }
    Check(popCount == COLS * ROWS && VisitedSum() == COLS * ROWS,
        "Fill: all 208 cells visited exactly once");
    Check(beforeEsp == afterEsp, "Fill: top-level ESP unchanged after full-grid recursion");

    Reset();
    // Seven full even rows plus six alternating single-cell connectors = 118.
    // Empty separator rows prevent shortcuts: traversal follows the whole snake.
    int pathCells = 0;
    for (int row = 0; row < ROWS; ++row) {
        if (row % 2 == 0) {
            for (int col = 0; col < COLS; ++col) { grid[row * COLS + col] = 5; ++pathCells; }
        } else {
            int col = (row / 2) % 2 == 0 ? COLS - 1 : 0;
            grid[row * COLS + col] = 5;
            ++pathCells;
        }
    }
    Match_Fill(0, 0, 5, 0);
    int exact = 1;
    for (int i = 0; i < COLS * ROWS; ++i) if (visited[i] != (grid[i] != EMPTY)) exact = 0;
    Check(pathCells == 118 && popCount == pathCells && VisitedSum() == pathCells && exact,
        "Fill: 118-cell one-wide serpentine path, no shortcuts or stack corruption");
}

static void TestResolve()
{
    Reset();
    score = 100;
    grid[0] = grid[1] = grid[2] = 1;
    Match_Resolve(1, 0);
    Check(Grid_IsEmpty() && score == 130, "Resolve: exactly 3 pop, nonzero score gains 30, grid empty");

    Reset();
    grid[0] = grid[1] = 1;
    grid[5 * COLS + 5] = 4; // A floater must remain when no successful pop occurs.
    BYTE before[COLS * ROWS];
    memcpy(before, grid, sizeof(grid));
    score = 77;
    Match_Resolve(0, 0);
    Check(!memcmp(before, grid, sizeof(grid)) && score == 77 && popCount == 2,
        "Resolve: 2-match leaves disconnected bubbles and score unchanged");

    Reset();
    grid[0] = grid[1] = grid[2] = grid[COLS] = grid[COLS + 1] = 3;
    Match_Resolve(0, 0);
    Check(Grid_IsEmpty() && score == 50, "Resolve: entire component larger than 3 removed");

    Reset();
    grid[0] = grid[COLS + 1] = grid[2 * COLS + 2] = 2;
    Match_Resolve(1, 1);
    Check(Occupied() == 3 && score == 0 && popCount == 1, "Resolve: diagonal-only bubbles cannot form a match");

    Reset();
    memset(grid, 5, sizeof(grid));
    Match_Resolve(15, 12);
    Check(Grid_IsEmpty() && score == 2080, "Resolve: full 208-cell pop scores exactly 2080");

    Reset();
    grid[0] = 2;
    memset(visited, 7, sizeof(visited));
    memcpy(before, grid, sizeof(grid));
    BYTE marks[COLS * ROWS];
    memcpy(marks, visited, sizeof(visited));
    popCount = 91;
    score = 100;
    const int invalid[][2] = {{-1,0},{0,-1},{COLS,0},{0,ROWS},{INT_MIN,INT_MAX},{INT_MAX,INT_MIN}};
    for (int i = 0; i < 6; ++i) Match_Resolve(invalid[i][0], invalid[i][1]);
    Match_Resolve(1, 1); // Empty selected cell.
    Check(!memcmp(before, grid, sizeof(grid)) && !memcmp(marks, visited, sizeof(visited)) && popCount == 91 && score == 100,
        "Resolve: empty and invalid starts preserve grid, score, visited and popCount");

    Reset();
    grid[0] = grid[1] = grid[2] = 1;
    Match_Resolve(0, 0);
    Match_Resolve(0, 0); // Already removed: cannot score twice.
    grid[0] = grid[1] = grid[2] = 4;
    Match_Resolve(2, 0);
    Check(Grid_IsEmpty() && score == 60, "Resolve: repeated calls clear stale marks and never double-score empty cells");
}

static void TestFloating()
{
    Reset();
    grid[0] = grid[1] = grid[2] = 1;
    for (int row = 0; row < 6; ++row) grid[row * COLS + 10] = (BYTE)(row % NUM_COLORS + 1);
    Match_Resolve(0, 0);
    int intact = 1;
    for (int row = 0; row < 6; ++row) if (grid[row * COLS + 10] != row % NUM_COLORS + 1) intact = 0;
    Check(intact && Occupied() == 6 && score == 30 && popCount == 6,
        "Floating: mixed-color chain to row 0 remains; popCount reused as connected count");

    Reset();
    score = 100;
    grid[0] = grid[1] = grid[2] = 2;
    grid[5 * COLS + 5] = 3;
    grid[5 * COLS + 6] = 4;
    Match_Resolve(1, 0);
    Check(Grid_IsEmpty() && score == 170 && popCount == 0,
        "Floating: pop 3 + drop 2 adds 70 to starting 100, no stale matched count");
    Match_Resolve(1, 0);
    Check(score == 170 && Grid_IsEmpty(), "Floating: repeated resolve on removed cells never scores twice");

    Reset();
    grid[4] = 2; // Keep this ceiling anchor; popping its stem releases two colors.
    grid[COLS + 4] = grid[2 * COLS + 4] = grid[3 * COLS + 4] = 1;
    grid[4 * COLS + 4] = 3;
    grid[4 * COLS + 5] = 4;
    Match_Resolve(4, 2);
    Check(Occupied() == 1 && grid[4] == 2 && score == 70 && popCount == 1,
        "Floating: popping support releases two bubbles while ceiling anchor remains");

    Reset();
    grid[0] = grid[1] = grid[2] = 5;
    grid[8] = 1; grid[9] = 2; grid[COLS + 8] = 3; grid[15] = 4;
    Match_Resolve(0, 0);
    Check(Occupied() == 4 && score == 30 && popCount == 4 && VisitedSum() == 4,
        "Floating: all nonempty top-row roots marked, overlapping roots not double-counted");
    grid[9] = 1; grid[COLS + 8] = 1;
    Match_Resolve(8, 0);
    Check(Occupied() == 1 && grid[15] == 4 && score == 60 && popCount == 1,
        "Floating: next successful match clears old connectivity marks");

    Reset();
    grid[0] = grid[1] = grid[2] = 1;
    grid[COLS + 3] = 5; // Only diagonal contact with the popped component.
    Match_Resolve(0, 0);
    Check(Grid_IsEmpty() && score == 50, "Floating: diagonal contact does not attach an orphan to the ceiling");

    Reset();
    grid[0] = grid[1] = 1;
    grid[12 * COLS + 15] = 5;
    score = 900;
    Match_Resolve(0, 0);
    Check(Occupied() == 3 && grid[12 * COLS + 15] == 5 && score == 900,
        "Floating: failed 2-match never invokes floating removal");
}

int main()
{
    TestFill();
    TestResolve();
    TestFloating();
    printf("Match tests: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
