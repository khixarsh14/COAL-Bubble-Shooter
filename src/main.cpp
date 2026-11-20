// Owner: Usman Khan.
// Concepts: program entry, console service calls, and clean preview exit.
#include "console.h"
#include <stdio.h>

int main()
{
    if (!Con_Init()) {
        fprintf(stderr, "Cannot initialize an 80x25 Windows console. Run in a console window.\n");
        return 1;
    }
    Con_DrawFrame();
    Con_WaitForExit();
    Con_Shutdown();
    return 0;
}
