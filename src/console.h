// Owner: Abdullah bin Faheem.
// Concepts: plain cdecl interfaces to Win32 console services.
#ifndef BUBBLE_CONSOLE_H
#define BUBBLE_CONSOLE_H
#include "game.h"

// Returns 1 on success, 0 if no suitable Windows console is available.
int __cdecl Con_Init();
void __cdecl Con_Shutdown();
void __cdecl Con_DrawFrame();
void __cdecl Con_DrawText(int x, int y, const char* s, int colorAttr);
void __cdecl Con_WaitForExit();

// Reserved services, not implemented or called in Milestone 1.
void __cdecl Con_DrawBubble(int col, int row, int color);
void __cdecl Con_DrawShot(int cellX, int cellY, int color);
void __cdecl Con_DrawAim(int index);
void __cdecl Con_DrawLauncher(int curColor);
void __cdecl Con_ReadKeys();
void __cdecl Con_Sleep(int ms);
int __cdecl Con_Rand(int n);
void __cdecl Con_Beep(int freq, int ms);
void __cdecl Con_ShowScreen(int which);
#endif
