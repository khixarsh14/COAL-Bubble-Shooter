// Owner: Abdullah bin Faheem.
// Concepts: Win32 cursor positioning, colors, console setup, and blocking input.
#include "console.h"
#include <conio.h>
#include <string.h>

static HANDLE outputHandle = INVALID_HANDLE_VALUE;
static CONSOLE_SCREEN_BUFFER_INFO savedInfo;
static CONSOLE_CURSOR_INFO savedCursor;
static char savedTitle[1024];
static int consoleReady = 0;

// Win32 services: see docs/OUT_OF_OUTLINE.md, Win32 console API.
int __cdecl Con_Init()
{
    outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(outputHandle, &savedInfo) ||
        !GetConsoleCursorInfo(outputHandle, &savedCursor))
        return 0;
    GetConsoleTitleA(savedTitle, sizeof(savedTitle));
    consoleReady = 1;
    COORD size = savedInfo.dwSize;
    if (size.X < 80) size.X = 80;
    if (size.Y < 25) size.Y = 25;
    SMALL_RECT window = {0, 0, 79, 24};
    CONSOLE_CURSOR_INFO cursor = savedCursor;
    cursor.bVisible = FALSE;
    DWORD written;
    COORD home = {0, 0};
    if (!SetConsoleScreenBufferSize(outputHandle, size) ||
        !SetConsoleWindowInfo(outputHandle, TRUE, &window) ||
        !SetConsoleCursorInfo(outputHandle, &cursor) ||
        !SetConsoleTitleA("Bubble Shooter - Milestone 1") ||
        !FillConsoleOutputCharacterA(outputHandle, ' ', size.X * size.Y, home, &written) ||
        !FillConsoleOutputAttribute(outputHandle, 7, size.X * size.Y, home, &written)) {
        Con_Shutdown();
        return 0;
    }
    return 1;
}

void __cdecl Con_Shutdown()
{
    if (!consoleReady) return;
    SetConsoleTextAttribute(outputHandle, savedInfo.wAttributes);
    SetConsoleCursorInfo(outputHandle, &savedCursor);
    SetConsoleTitleA(savedTitle);
    COORD belowFrame = {0, 22};
    SetConsoleCursorPosition(outputHandle, belowFrame);
    consoleReady = 0;
}

void __cdecl Con_DrawText(int x, int y, const char* s, int colorAttr)
{
    COORD position = {(SHORT)x, (SHORT)y};
    DWORD written;
    SetConsoleCursorPosition(outputHandle, position);
    SetConsoleTextAttribute(outputHandle, (WORD)colorAttr);
    WriteConsoleA(outputHandle, s, (DWORD)strlen(s), &written, 0);
}

void __cdecl Con_DrawFrame()
{
    const int right = COLS * 2 + 1;
    const int bottom = ROWS + 4;
    Con_DrawText(1, 0, "BUBBLE SHOOTER     x86 Assembly (inline __asm + C++)", 15);
    Con_DrawText(0, 1, "+--------------------------------+", 7);
    for (int y = 2; y < bottom; ++y) {
        Con_DrawText(0, y, "|", 7);
        Con_DrawText(right, y, "|", 7);
    }
    Con_DrawText(0, bottom, "+--------------------------------+", 7);
    // Row 0 starts at screen y=2: danger row 11 is screen y=13.
    Con_DrawText(1, 2 + DANGER_ROW, "- - - - DANGER LINE - - - - - - - ", 12);
    Con_DrawText(15, bottom - 2, "^", 7);
    Con_DrawText(14, bottom - 1, "[O]", 7);
    // Static placeholders, not a score calculation or a next-bubble system.
    Con_DrawText(39, 2, "SCORE : 0000", 7);
    Con_DrawText(39, 3, "LEVEL : 1", 7);
    Con_DrawText(39, 4, "HIGH  : 0000", 7);
    Con_DrawText(39, 6, "NEXT    : -", 7);
    Con_DrawText(39, 7, "DROP IN : - shots", 7);
    Con_DrawText(39, 9, "CONTROLS", 15);
    Con_DrawText(39, 10, "<- / ->  aim", 7);
    Con_DrawText(39, 11, "SPACE    shoot", 7);
    Con_DrawText(39, 12, "P        pause", 7);
    Con_DrawText(39, 13, "ESC      menu", 7);
    Con_DrawText(1, bottom + 1, "Match 3 or more bubbles of the same color to pop them!", 7);
    Con_DrawText(1, bottom + 3, "Milestone 1: static frame only. ESC exits this preview.", 14);
}

// Blocking preview input: see docs/OUT_OF_OUTLINE.md, Console key wait.
void __cdecl Con_WaitForExit()
{
    while (_getch() != 27) {
        // No aim, shooting, pause, or menu handling in this preview.
    }
}
