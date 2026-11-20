// Owner: Khizar Sheikh (Milestone 2 regression check).
// Concepts: isolated Win32 child-console test; no changes to the game entry point.
// See docs/OUT_OF_OUTLINE.md, Preview regression harness.
#include "../src/game.h"
#include <stdio.h>
#include <string.h>

static int HasText(HANDLE screen, int x, int y, const char* expected)
{
    char actual[80] = {0};
    DWORD read = 0;
    COORD position = {(SHORT)x, (SHORT)y};
    DWORD count = (DWORD)strlen(expected);
    return ReadConsoleOutputCharacterA(screen, actual, count, position, &read) &&
        read == count && memcmp(actual, expected, count) == 0;
}

int main()
{
    STARTUPINFOA startup = {0};
    PROCESS_INFORMATION child = {0};
    startup.cb = sizeof(startup);
    startup.dwFlags = STARTF_USESHOWWINDOW;
    startup.wShowWindow = SW_HIDE;
    char command[] = "bin\\BubbleShooter.exe";
    if (!CreateProcessA(0, command, 0, 0, FALSE, CREATE_NEW_CONSOLE, 0, 0, &startup, &child)) {
        printf("FAIL: cannot launch preview (%lu)\n", GetLastError());
        return 1;
    }
    FreeConsole();
    int attached = 0;
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
    int drawn = 0;
    for (int attempt = 0; attempt < 100 && !drawn && screen != INVALID_HANDLE_VALUE; ++attempt) {
        drawn = HasText(screen, 1, 0, "BUBBLE SHOOTER") &&
            HasText(screen, 0, 1, "+--------------------------------+") &&
            HasText(screen, 0, 17, "+--------------------------------+") &&
            HasText(screen, 1, 13, "- - - - DANGER LINE") &&
            HasText(screen, 39, 2, "SCORE : 0000") &&
            HasText(screen, 39, 9, "CONTROLS") &&
            HasText(screen, 1, 20, "Milestone 1: static frame only. ESC exits this preview.");
        if (!drawn) Sleep(30);
    }
    CONSOLE_CURSOR_INFO cursor = {0};
    int hidden = drawn && GetConsoleCursorInfo(screen, &cursor) && !cursor.bVisible;
    INPUT_RECORD escape = {0};
    escape.EventType = KEY_EVENT;
    escape.Event.KeyEvent.bKeyDown = TRUE;
    escape.Event.KeyEvent.wRepeatCount = 1;
    escape.Event.KeyEvent.wVirtualKeyCode = VK_ESCAPE;
    escape.Event.KeyEvent.uChar.AsciiChar = 27;
    DWORD written = 0, exitCode = 1;
    int sent = drawn && WriteConsoleInputA(input, &escape, 1, &written) && written == 1;
    int exited = sent && WaitForSingleObject(child.hProcess, 5000) == WAIT_OBJECT_0 &&
        GetExitCodeProcess(child.hProcess, &exitCode) && exitCode == 0;
    int restored = exited && GetConsoleCursorInfo(screen, &cursor) && cursor.bVisible;
    if (!exited) {
        TerminateProcess(child.hProcess, 1); // Clean up only the child created by this test.
        WaitForSingleObject(child.hProcess, 1000);
    }
    if (screen != INVALID_HANDLE_VALUE) CloseHandle(screen);
    if (input != INVALID_HANDLE_VALUE) CloseHandle(input);
    if (attached) FreeConsole();
    CloseHandle(child.hThread);
    CloseHandle(child.hProcess);
    printf("%s: preview frame text and danger line\n", drawn ? "PASS" : "FAIL");
    printf("%s: cursor hidden while preview waits\n", hidden ? "PASS" : "FAIL");
    printf("%s: ESC exits preview with code 0\n", exited ? "PASS" : "FAIL");
    printf("%s: cursor visible after shutdown\n", restored ? "PASS" : "FAIL");
    return drawn && hidden && exited && restored ? 0 : 1;
}
