// Owner: Usman Khan (integration proof only; separate from the game).
// Concepts: x86 inline MOV and ADD, function parameters, and integer return.
// The macro prevents a second main when src\*.cpp builds the preview.
#ifdef BUBBLE_ASM_TEST
#include "game.h"
#include <stdio.h>

int __cdecl Asm_Add(int first, int second)
{
    int result;
    // Add two small signed integers through EAX; leave ESP and EBP unchanged.
    __asm {
        mov eax, first   // Copy the first input into the 32-bit EAX register.
        add eax, second  // Add the second input to EAX.
        mov result, eax  // Copy the sum into the C++ local variable.
    }
    return result;
}

int main()
{
    int result = Asm_Add(7, 5);
    printf("MSVC x86 inline assembly: 7 + 5 = %d\n", result);
    return result == 12 ? 0 : 1;
}
#endif
