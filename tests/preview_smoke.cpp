// Live console integration test using input records sent to a child process.
#include "../src/game.h"
#include <stdio.h>
#include <string.h>

static bool HasText(HANDLE screen, int x, int y, const char* expected)
{
    char actual[80] = {};
    DWORD read;
    COORD position = {(SHORT)x, (SHORT)y};
    DWORD count = (DWORD)strlen(expected);
    return ReadConsoleOutputCharacterA(screen, actual, count, position, &read) &&
        read == count && memcmp(actual, expected, count) == 0;
}

static bool WaitText(HANDLE screen, int x, int y, const char* expected)
{
    for (int attempt = 0; attempt < 150; ++attempt) {
        if (HasText(screen, x, y, expected)) return true;
        Sleep(20);
    }
    return false;
}

static bool Tap(HANDLE input, WORD key)
{
    INPUT_RECORD events[2] = {};
    events[0].EventType = KEY_EVENT;
    events[0].Event.KeyEvent.bKeyDown = TRUE;
    events[0].Event.KeyEvent.wRepeatCount = 1;
    events[0].Event.KeyEvent.wVirtualKeyCode = key;
    events[1] = events[0];
    events[1].Event.KeyEvent.bKeyDown = FALSE;
    DWORD written;
    return WriteConsoleInputA(input, events, 2, &written) && written == 2;
}

int main()
{
    STARTUPINFOA startup = {};
    PROCESS_INFORMATION child = {};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    char command[] = "bin\\BubbleShooter.exe";
    if (!CreateProcessA(0, command, 0, 0, FALSE, CREATE_NEW_CONSOLE, 0, 0, &startup, &child)) {
        printf("FAIL: cannot launch game (%lu)\n", GetLastError());
        return 1;
    }

    FreeConsole();
    bool attached = false;
    for (int attempt = 0; attempt < 100 && !attached; ++attempt) {
        attached = AttachConsole(child.dwProcessId) != 0;
        if (!attached) Sleep(30);
    }
    HANDLE screen = INVALID_HANDLE_VALUE, input = INVALID_HANDLE_VALUE;
    if (attached) {
        screen = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
        input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
    }
    const bool menu = screen != INVALID_HANDLE_VALUE && WaitText(screen, 2, 3, "1  Start game");
    CONSOLE_CURSOR_INFO cursor = {};
    const bool hidden = menu && GetConsoleCursorInfo(screen, &cursor) && !cursor.bVisible;
    const bool help = menu && Tap(input, '2') && WaitText(screen, 2, 3, "Aim with Left/Right") &&
        Tap(input, VK_ESCAPE) && WaitText(screen, 2, 3, "1  Start game");
    const bool board = help && Tap(input, '1') && WaitText(screen, 49, 7, "7") &&
        HasText(screen, 15, 16, "O") &&
        HasText(screen, 1, 2, "O") && HasText(screen, 1, 13, "- ") &&
        HasText(screen, 49, 7, "7") && HasText(screen, 15, 15, "^");
    bool shot = board && Tap(input, VK_SPACE) && WaitText(screen, 15, 14, "O");
    if (shot) {
        for (int attempt = 0; attempt < 150 && !HasText(screen, 15, 7, "O"); ++attempt)
            Sleep(20);
        shot = HasText(screen, 15, 14, "  ") && HasText(screen, 15, 13, "- ") &&
            HasText(screen, 15, 15, "^");
    }
    const bool pause = shot && Tap(input, 'P') && WaitText(screen, 1, 18, "PAUSED") &&
        Tap(input, 'P') && WaitText(screen, 1, 18, "      ");
    DWORD exitCode = 1;
    const bool exited = pause && Tap(input, VK_ESCAPE) && WaitText(screen, 2, 3, "1  Start game") &&
        Tap(input, '3') && WaitForSingleObject(child.hProcess, 5000) == WAIT_OBJECT_0 &&
        GetExitCodeProcess(child.hProcess, &exitCode) && exitCode == 0;
    const bool restored = exited && GetConsoleCursorInfo(screen, &cursor) && cursor.bVisible;

    if (!exited) {
        TerminateProcess(child.hProcess, 1);
        WaitForSingleObject(child.hProcess, 1000);
    }
    if (screen != INVALID_HANDLE_VALUE) CloseHandle(screen);
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (attached) FreeConsole();
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    printf("%s: menu and hidden cursor\n", hidden ? "PASS" : "FAIL");
    printf("%s: help screen returns to menu\n", help ? "PASS" : "FAIL");
    printf("%s: live board, launcher, aim, danger line and drop HUD\n", board ? "PASS" : "FAIL");
    printf("%s: moving shot restores traversed cells\n", shot ? "PASS" : "FAIL");
    printf("%s: pause and resume\n", pause ? "PASS" : "FAIL");
    printf("%s: menu exit with code 0 and restored cursor\n", restored ? "PASS" : "FAIL");
    return hidden && help && board && shot && pause && restored ? 0 : 1;
}
