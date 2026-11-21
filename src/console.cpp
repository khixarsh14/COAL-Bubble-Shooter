// Concepts: nonblocking Win32 input and rendering with assembly cell/HUD updates.
#include "console.h"
#include <stdlib.h>
#include <string.h>

// Platform calls are described in docs/OUT_OF_OUTLINE.md, Live console events and incremental drawing.
enum { GRID_X = 1, GRID_Y = 2, FRAME_BOTTOM = ROWS + 4, CELLS = COLS * ROWS };
static HANDLE outputHandle, inputHandle;
static CONSOLE_SCREEN_BUFFER_INFO savedInfo;
static CONSOLE_CURSOR_INFO savedCursor;
static DWORD savedInputMode;
static char savedTitle[1024];
static int consoleReady;
static BYTE down[256];
static BYTE displayed[CELLS];
static int shownState = -1, shownAim, shownLauncher, shownLane, shownLaneColor;
static int shownScore, shownLevel, shownHigh, shownDrop, shownNext;
static const WORD palette[] = {7, 12, 10, 11, 14, 13};

int __cdecl Con_Init()
{
    outputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(outputHandle, &savedInfo) ||
        !GetConsoleCursorInfo(outputHandle, &savedCursor) ||
        !GetConsoleMode(inputHandle, &savedInputMode)) return 0;
    GetConsoleTitleA(savedTitle, sizeof(savedTitle));
    consoleReady = 1;
    COORD size = savedInfo.dwSize;
    if (size.X < 80) size.X = 80;
    if (size.Y < 25) size.Y = 25;
    SMALL_RECT window = {0, 0, 79, 24};
    CONSOLE_CURSOR_INFO cursor = savedCursor;
    cursor.bVisible = FALSE;
    if (!SetConsoleScreenBufferSize(outputHandle, size) ||
        !SetConsoleWindowInfo(outputHandle, TRUE, &window) ||
        !SetConsoleCursorInfo(outputHandle, &cursor) ||
        !SetConsoleMode(inputHandle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT) ||
        !SetConsoleTitleA("Bubble Shooter")) {
        Con_Shutdown();
        return 0;
    }
    srand(GetTickCount());           // See docs/OUT_OF_OUTLINE.md, Pseudorandom colors.
    return 1;
}

void __cdecl Con_Shutdown()
{
    if (!consoleReady) return;
    SetConsoleMode(inputHandle, savedInputMode);
    SetConsoleWindowInfo(outputHandle, TRUE, &savedInfo.srWindow);
    SetConsoleCursorPosition(outputHandle, savedInfo.dwCursorPosition);
    SetConsoleScreenBufferSize(outputHandle, savedInfo.dwSize);
    SetConsoleTextAttribute(outputHandle, savedInfo.wAttributes);
    SetConsoleCursorInfo(outputHandle, &savedCursor);
    SetConsoleTitleA(savedTitle);
    consoleReady = 0;
}

void __cdecl Con_DrawText(int x, int y, const char* s, int colorAttr)
{
    COORD position = {(SHORT)x, (SHORT)y};
    DWORD written;
    DWORD length = (DWORD)strlen(s);
    WriteConsoleOutputCharacterA(outputHandle, s, length, position, &written);
    FillConsoleOutputAttribute(outputHandle, (WORD)colorAttr, length, position, &written);
}

static void __cdecl ClearScreen()
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(outputHandle, &info)) return;
    COORD home = {0, 0};
    DWORD written;
    DWORD size = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacterA(outputHandle, ' ', size, home, &written);
    FillConsoleOutputAttribute(outputHandle, 7, size, home, &written);
}

struct TextLine { int x, y; const char* text; int color; };
static const char border[] = "+--------------------------------+", vertical[] = "|";
static const TextLine frame[] = {
    {1, 0, "BUBBLE SHOOTER", 15}, {0, 1, border, 7}, {0, FRAME_BOTTOM, border, 7},
    {39, 2, "SCORE", 7}, {39, 3, "LEVEL", 7}, {39, 4, "HIGH", 7},
    {39, 6, "NEXT", 7}, {39, 7, "DROP IN", 7},
    {39, 9, "Left / Right  Aim", 7}, {39, 10, "Space         Shoot", 7},
    {39, 11, "P             Pause", 7}, {39, 12, "Esc           Menu", 7},
    {39, GRID_Y + DANGER_ROW, "< Danger line", 12}
};
enum { FRAME_LINES = sizeof(frame) / sizeof(frame[0]), LINE_STRIDE = sizeof(TextLine) };

static void __cdecl DrawLines(const TextLine* lines, int count)
{
    // Shared renderer for fixed console text, using the four cdecl fields.
    __asm {
        mov esi, lines
        mov ebx, count
        test ebx, ebx
        jz lines_done
    lines_next:
        push dword ptr [esi + 12]
        push dword ptr [esi + 8]
        push dword ptr [esi + 4]
        push dword ptr [esi]
        call Con_DrawText
        add esp, 16
        add esi, LINE_STRIDE
        dec ebx
        jnz lines_next
    lines_done:
    }
}

void __cdecl Con_DrawFrame()
{
    // Draw the static labels and both walls once per entry into the playfield.
    __asm {
        push FRAME_LINES
        lea eax, frame
        push eax
        call DrawLines
        add esp, 8
        mov ebx, GRID_Y
        lea esi, vertical
    frame_wall:
        push 7
        push esi
        push ebx
        push 0
        call Con_DrawText
        add esp, 16
        push 7
        push esi
        push ebx
        push COLS * 2 + 1
        call Con_DrawText
        add esp, 16
        inc ebx
        cmp ebx, FRAME_BOTTOM
        jl frame_wall
    }
}

static const char bubbleGlyph[] = "O ", dangerGlyph[] = "- ", blankCell[] = "  ";
static const char aimGlyph[] = "^ ", launcherGlyph[] = "[O]";

void __cdecl Con_DrawBubble(int col, int row, int color)
{
    // Grid, moving shot, and erase use the same cell-to-console mapping.
    __asm {
        mov ecx, color
        cmp ecx, NUM_COLORS
        ja cell_done
        mov edx, row
        cmp edx, LAUNCH_ROW
        ja cell_done
        mov eax, col
        cmp eax, COLS
        jae cell_done
        lea eax, [eax * 2 + GRID_X]
        movzx ecx, word ptr palette[ecx * 2]
        lea esi, bubbleGlyph
        cmp color, EMPTY
        jne cell_emit
        lea esi, blankCell
        cmp edx, DANGER_ROW
        jne cell_emit
        lea esi, dangerGlyph
    cell_emit:
        add edx, GRID_Y
        push ecx
        push esi
        push edx
        push eax
        call Con_DrawText
        add esp, 16
    cell_done:
    }
}

void __cdecl Con_DrawShot(int cellX, int cellY, int color)
{
    Con_DrawBubble(cellX, cellY, color);
}

void __cdecl Con_DrawAim(int index)
{
    // Map the nine discrete aim indices onto the launcher lane.
    __asm {
        mov eax, index
        cmp eax, NUM_ANGLES
        jae marker_done
        lea eax, [eax * 2 + GRID_X + (LAUNCH_COL - NUM_ANGLES / 2) * 2]
        lea edx, aimGlyph
        push 7
        push edx
        push GRID_Y + LAUNCH_ROW
        push eax
        call Con_DrawText
        add esp, 16
    marker_done:
    }
}

void __cdecl Con_DrawLauncher(int color)
{
    // Translate the current color to a console attribute.
    __asm {
        mov eax, color
        cmp eax, NUM_COLORS
        ja launcher_done
        movzx eax, word ptr palette[eax * 2]
        lea edx, launcherGlyph
        push eax
        push edx
        push GRID_Y + LAUNCH_ROW + 1
        push GRID_X + 2 * LAUNCH_COL - 1
        call Con_DrawText
        add esp, 16
    launcher_done:
    }
}

enum { NO_EVENT = -1, SKIP_EVENT = -2, LOST_FOCUS = -3, DOWN_EVENT = 256 };
struct KeyBinding { int virtualKey, mask; };
static const KeyBinding bindings[] = {
    {VK_LEFT, KEY_LEFT}, {VK_RIGHT, KEY_RIGHT}, {VK_SPACE, KEY_SPACE},
    {'P', KEY_P}, {VK_ESCAPE, KEY_ESC}, {'1', KEY_1}, {'2', KEY_2}, {'3', KEY_3},
    {VK_NUMPAD1, KEY_1}, {VK_NUMPAD2, KEY_2}, {VK_NUMPAD3, KEY_3}
};
enum { BINDING_COUNT = sizeof(bindings) / sizeof(bindings[0]), BINDING_STRIDE = sizeof(bindings[0]) };

static int __cdecl ReadKeyEvent()
{
    DWORD pending = 0, read;
    INPUT_RECORD event;
    if (!GetNumberOfConsoleInputEvents(inputHandle, &pending) || !pending ||
        !ReadConsoleInputA(inputHandle, &event, 1, &read) || !read) return NO_EVENT;
    if (event.EventType == FOCUS_EVENT && !event.Event.FocusEvent.bSetFocus) return LOST_FOCUS;
    if (event.EventType != KEY_EVENT || event.Event.KeyEvent.wVirtualKeyCode >= 256) return SKIP_EVENT;
    return event.Event.KeyEvent.wVirtualKeyCode + (event.Event.KeyEvent.bKeyDown ? DOWN_EVENT : 0);
}

void __cdecl Con_ReadKeys()
{
    // Translate native events into held actions and one fresh-key pulse per frame.
    __asm {
        xor ebx, ebx
        mov esi, 64                 // Bound queue work even when input is very busy.
    keys_event:
        call ReadKeyEvent
        cmp eax, NO_EVENT
        je keys_bindings
        cmp eax, LOST_FOCUS
        je keys_clear
        cmp eax, SKIP_EVENT
        je keys_next
        mov ecx, eax
        and ecx, DOWN_EVENT - 1
        test eax, DOWN_EVENT
        jz keys_release
        cmp byte ptr down[ecx], 0
        jne keys_next               // Ignore OS repeat. Arrows retain their held bits.
        mov byte ptr down[ecx], 1
        mov ebx, KEY_ANY
        jmp keys_bindings           // Keep later raw transitions for the next frame.
    keys_release:
        cmp byte ptr down[ecx], 0
        je keys_next
        mov byte ptr down[ecx], 0
        jmp keys_bindings           // Publish the release before a queued second tap.
    keys_clear:
        lea edi, down
        xor eax, eax
        mov ecx, DOWN_EVENT / 4
        cld
        rep stosd
        jmp keys_bindings
    keys_next:
        dec esi
        jnz keys_event
    keys_bindings:
        lea esi, bindings
        mov edi, BINDING_COUNT
    keys_binding:
        mov ecx, [esi]
        cmp byte ptr down[ecx], 0
        je keys_binding_next
        or ebx, [esi + 4]
    keys_binding_next:
        add esi, BINDING_STRIDE
        dec edi
        jnz keys_binding
        mov keyBits, ebx
    }
}

void __cdecl Con_Sleep(int ms) { Sleep(ms); }

int __cdecl Con_Rand(int n) { return n > 0 ? rand() % n : 0; }

static void __cdecl DrawNumber(int x, int y, int value)
{
    // Pad the decimal text to a fixed width and write it in one operation.
    __asm {
        lea esi, numBuf
        push esi
        push value
        call Num_ToString
        add esp, 8
        mov edi, esi
        xor eax, eax
        mov ecx, 12
        cld
        repne scasb
        dec edi
        mov al, ' '
        rep stosb
        mov byte ptr [edi], 0
        push 15
        push esi
        push y
        push x
        call Con_DrawText
        add esp, 16
    }
}

static const TextLine menuLines[] = {
    {2, 3, "1  Start game", 7}, {2, 4, "2  How to play", 7}, {2, 5, "3  Exit", 7}
};
static const TextLine helpLines[] = {
    {2, 3, "Aim with Left/Right and shoot with Space.", 7},
    {2, 4, "Connect 3 of the same color up/down/left/right.", 7},
    {2, 5, "Clear the board before bubbles reach the danger line.", 7},
    {2, 7, "P pauses. Esc returns to the menu.", 7}
};
static const TextLine lossLines[] = {
    {2, 3, "GAME OVER", 12}, {2, 5, "FINAL SCORE", 7},
    {2, 7, "Press any key for the menu.", 7}
};
static const char title[] = "BUBBLE SHOOTER";
enum { MENU_LINES = sizeof(menuLines) / sizeof(menuLines[0]),
    HELP_LINES = sizeof(helpLines) / sizeof(helpLines[0]),
    LOSS_LINES = sizeof(lossLines) / sizeof(lossLines[0]) };

void __cdecl Con_ShowScreen(int which)
{
    // Select the requested text layout, then append the final score if applicable.
    __asm {
        call ClearScreen
        lea eax, title
        push 15
        push eax
        push 0
        push 1
        call Con_DrawText
        add esp, 16
        mov eax, which
        cmp eax, STATE_MENU
        je screen_menu
        cmp eax, STATE_HELP
        je screen_help
        cmp eax, STATE_GAMEOVER
        jne screen_done
        push LOSS_LINES
        lea eax, lossLines
        jmp screen_lines
    screen_menu:
        push MENU_LINES
        lea eax, menuLines
        jmp screen_lines
    screen_help:
        push HELP_LINES
        lea eax, helpLines
    screen_lines:
        push eax
        call DrawLines
        add esp, 8
        cmp which, STATE_GAMEOVER
        jne screen_done
        push score
        push 5
        push 15
        call DrawNumber
        add esp, 12
    screen_done:
    }
}

static void __cdecl DrawBoard()
{
    int overlayCol, overlayRow;
    // Compose the shot over the grid, then draw only cells with changed colors.
    __asm {
        mov overlayCol, -1
        mov overlayRow, -1
        cmp shotActive, 0
        je board_start
        mov eax, shotX
        sar eax, 8
        mov overlayCol, eax
        mov eax, shotY
        sar eax, 8
        mov overlayRow, eax
    board_start:
        xor ebx, ebx                 // Linear index covers each cell exactly once.
    board_cell:
        mov esi, ebx
        and esi, COLS - 1
        mov edi, ebx
        shr edi, 4
        movzx eax, byte ptr grid[ebx]
        cmp esi, overlayCol
        jne board_compare
        cmp edi, overlayRow
        jne board_compare
        mov eax, curColor
    board_compare:
        cmp byte ptr displayed[ebx], al
        je board_next
        mov byte ptr displayed[ebx], al
        push eax
        push edi
        push esi
        cmp esi, overlayCol
        jne board_draw_cell
        cmp edi, overlayRow
        jne board_draw_cell
        call Con_DrawShot
        jmp board_draw_done
    board_draw_cell:
        call Con_DrawBubble
    board_draw_done:
        add esp, 12
    board_next:
        inc ebx
        cmp ebx, CELLS
        jb board_cell
    }
}

static void __cdecl DrawLauncherArea()
{
    // Redraw the aim and the flying bubble in the lane as a single layer.
    __asm {
        mov ebx, -1                  // Current shot column in the lane, or absent.
        xor edi, edi                 // Current lane color.
        cmp shotActive, 0
        je lane_compare
        mov eax, shotY
        sar eax, 8
        cmp eax, LAUNCH_ROW
        jne lane_compare
        mov ebx, shotX
        sar ebx, 8
        mov edi, curColor
    lane_compare:
        mov esi, aimIndex
        cmp shownAim, esi
        jne lane_redraw
        cmp shownLane, ebx
        jne lane_redraw
        cmp shownLaneColor, edi
        je lane_launcher
    lane_redraw:
        mov eax, shownAim
        cmp eax, 0
        jl lane_erase_shot
        add eax, LAUNCH_COL - NUM_ANGLES / 2
        push 0
        push LAUNCH_ROW
        push eax
        call Con_DrawBubble
        add esp, 12
    lane_erase_shot:
        cmp shownLane, 0
        jl lane_draw_aim
        push 0
        push LAUNCH_ROW
        push shownLane
        call Con_DrawShot
        add esp, 12
    lane_draw_aim:
        mov shownAim, esi
        mov shownLane, ebx
        mov shownLaneColor, edi
        lea eax, [esi + LAUNCH_COL - NUM_ANGLES / 2]
        cmp eax, ebx
        je lane_draw_shot
        push esi
        call Con_DrawAim
        add esp, 4
    lane_draw_shot:
        cmp ebx, 0
        jl lane_launcher
        push edi
        push LAUNCH_ROW
        push ebx
        call Con_DrawShot
        add esp, 12
    lane_launcher:
        mov eax, curColor
        cmp eax, shownLauncher
        je lane_done
        mov shownLauncher, eax
        push eax
        call Con_DrawLauncher
        add esp, 4
    lane_done:
    }
}

struct HudNumber { int* value; int* previous; int row; };
static const HudNumber hud[] = {
    { &score, &shownScore, 2 }, { &level, &shownLevel, 3 },
    { &highScore, &shownHigh, 4 }, { &shotsUntilDrop, &shownDrop, 7 }
};
static const char nextGlyph[] = "O";
enum { HUD_COUNT = sizeof(hud) / sizeof(hud[0]), HUD_STRIDE = sizeof(hud[0]) };

static void __cdecl DrawHud()
{
    // Walk the HUD fields and redraw only numbers or colors that have changed.
    __asm {
        lea esi, hud
        mov ebx, HUD_COUNT
    hud_field:
        mov eax, [esi]
        mov eax, [eax]
        mov edx, [esi + 4]
        cmp eax, [edx]
        je hud_next_field
        mov [edx], eax
        push eax
        push dword ptr [esi + 8]
        push 49
        call DrawNumber
        add esp, 12
    hud_next_field:
        add esi, HUD_STRIDE
        dec ebx
        jnz hud_field
        mov eax, nextColor
        cmp shownNext, eax
        je hud_done
        mov shownNext, eax
        movzx edx, word ptr palette[eax * 2]
        push edx
        lea eax, nextGlyph
        push eax
        push 6
        push 49
        call Con_DrawText
        add esp, 16
    hud_done:
    }
}

static const char pauseText[] = "PAUSED - P resumes", clearPause[] = "                  ";

void __cdecl Con_Render()
{
    // Gameplay and pause share one frame. Invalidate the caches on a new round.
    __asm {
        mov ebx, gameState
        cmp ebx, shownState
        je render_existing
        cmp ebx, STATE_PLAYING
        je render_play
        cmp ebx, STATE_PAUSED
        je render_play
        push ebx
        call Con_ShowScreen
        add esp, 4
        jmp render_state
    render_play:
        cmp shownState, STATE_PLAYING
        je render_pause_label
        cmp shownState, STATE_PAUSED
        je render_pause_label
        call ClearScreen
        call Con_DrawFrame
        lea edi, displayed
        mov eax, -1
        mov ecx, CELLS
        cld
        rep stosb
        mov shownAim, eax
        mov shownLauncher, eax
        mov shownLane, eax
        mov shownLaneColor, eax
        mov shownScore, eax
        mov shownHigh, eax
        mov shownLevel, eax
        mov shownDrop, eax
        mov shownNext, eax
    render_pause_label:
        lea eax, clearPause
        cmp ebx, STATE_PAUSED
        jne render_label
        lea eax, pauseText
    render_label:
        push 14
        push eax
        push FRAME_BOTTOM + 1
        push 1
        call Con_DrawText
        add esp, 16
    render_state:
        mov shownState, ebx
    render_existing:
        cmp ebx, STATE_PLAYING
        je render_game
        cmp ebx, STATE_PAUSED
        jne render_done
    render_game:
        call DrawBoard
        call DrawLauncherArea
        call DrawHud
    render_done:
    }
}
