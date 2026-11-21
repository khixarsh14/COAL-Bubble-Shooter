// Concepts: integer scoring, level and drop thresholds, decimal division.
#include "game.h"

void __cdecl Score_AddPop(int count)
{
    // Available to callers that own an award. Match_Resolve awards its own.
    __asm {
        mov eax, count
        test eax, eax
        jle pop_score_done
        imul eax, POINTS_POP
        add score, eax
    pop_score_done:
    }
}

void __cdecl Score_AddDrop(int count)
{
    // Available to callers that own an award. Match_DropFloating awards its own.
    __asm {
        mov eax, count
        test eax, eax
        jle drop_score_done
        imul eax, POINTS_DROP
        add score, eax
    drop_score_done:
    }
}

void __cdecl Score_UpdateDrop(int popped)
{
    // A pop resets the interval. Only a miss can shift and refill the ceiling.
    __asm {
        cmp popped, 0
        jne drop_reset
        dec shotsUntilDrop
        jg drop_done
        call Grid_ShiftDown
        call Grid_AddTopRow
    drop_reset:
        mov eax, 8
        sub eax, level
        cmp eax, 3
        jge drop_store
        mov eax, 3
    drop_store:
        mov shotsUntilDrop, eax
    drop_done:
    }
}

void __cdecl Score_UpdateLevel()
{
    // An empty grid advances one level and refreshes the interval without a shift.
    __asm {
        call Grid_IsEmpty
        test eax, eax
        jz level_done
        cmp level, 7FFFFFFFh
        je level_refill
        inc level
    level_refill:
        call Grid_Init
        push 1
        call Score_UpdateDrop
        add esp, 4
    level_done:
    }
}

void __cdecl Num_ToString(int value, char* bufferAddress)
{
    // Convert signed int32. After NEG, even INT_MIN is a valid unsigned magnitude.
    __asm {
        mov edi, bufferAddress
        mov eax, value
        test eax, eax
        jns number_magnitude
        mov byte ptr [edi], '-'
        inc edi
        neg eax
    number_magnitude:
        xor ecx, ecx
        mov ebx, 10
    number_divide:
        xor edx, edx
        div ebx                      // EDX = digit, EAX = remaining magnitude.
        push edx
        inc ecx
        test eax, eax
        jnz number_divide
    number_write:
        pop edx                      // Last remainder is the most significant digit.
        add dl, '0'
        mov byte ptr [edi], dl
        inc edi
        dec ecx
        jnz number_write
        mov byte ptr [edi], 0
    }
}
