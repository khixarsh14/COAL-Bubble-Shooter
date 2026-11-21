// Concepts: console lifetime and nonblocking frame loop.
#include "console.h"
#include <stdio.h>

int main()
{
    if (!Con_Init()) {
        fprintf(stderr, "Cannot initialize an 80x25 Windows console. Run in a console window.\n");
        return 1;
    }
    Con_Render();
    while (gameState != STATE_EXIT) {
        Con_ReadKeys();
        Game_Update();
        if (gameState == STATE_EXIT) break;
        Con_Render();
        Con_Sleep(30);
    }
    Con_Shutdown();
    return 0;
}
