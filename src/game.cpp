// Concepts: state dispatch, input edges, and one transaction per landed shot.
#include "game.h"
#include "console.h"

static int previousKeys;
static int heldAim;
static DWORD lastAimTick;
enum { AIM_REPEAT_MS = 120 };

void __cdecl Game_Reset()
{
    // Start a fresh run. Retain the session record and held-key history.
    __asm {
        xor eax, eax
        mov score, eax
        mov heldAim, eax
        mov shotActive, eax
        mov shotX, eax
        mov shotY, eax
        mov shotDX, eax
        mov shotDY, eax
        mov aimIndex, NUM_ANGLES / 2
        mov lastCol, -1
        mov lastRow, -1
        push NUM_COLORS
        call Con_Rand
        add esp, 4
        inc eax
        mov curColor, eax
        push NUM_COLORS
        call Con_Rand
        add esp, 4
        inc eax
        mov nextColor, eax
        push 1
        call Score_UpdateDrop
        add esp, 4
        call Grid_Init
        mov gameState, STATE_PLAYING
    }
}

void __cdecl Game_Update()
{
    Game_UpdateAt(GetTickCount());
}

void __cdecl Game_UpdateAt(DWORD now)
{
    // ESI = action edges. EBX = held keys until the playing handler steps a shot.
    __asm {
        mov ebx, keyBits
        mov esi, previousKeys
        not esi
        and esi, ebx
        mov previousKeys, ebx

        mov eax, gameState
        cmp eax, STATE_PLAYING
        je update_play
        cmp eax, STATE_MENU
        je update_menu
        cmp eax, STATE_PAUSED
        je update_pause
        cmp eax, STATE_HELP
        je update_help
        cmp eax, STATE_GAMEOVER
        je update_finished
        cmp eax, STATE_WIN
        je update_finished
        jmp update_done

    update_menu:
        test esi, KEY_ESC | KEY_3
        jnz update_exit
        test esi, KEY_2
        jnz update_open_help
        test esi, KEY_1
        jz update_done
        call Game_Reset
        jmp update_done
    update_open_help:
        mov gameState, STATE_HELP
        jmp update_done
    update_help:
        test esi, KEY_ESC
        jnz update_to_menu
        jmp update_done
    update_finished:
        test ebx, KEY_ANY             // Console reports a fresh keydown pulse.
        jnz update_to_menu
        jmp update_done
    update_pause:
        test esi, KEY_ESC
        jnz update_to_menu
        test esi, KEY_P
        jz update_done
        mov gameState, STATE_PLAYING
        jmp update_done

    update_play:
        test esi, KEY_ESC
        jnz update_to_menu
        test esi, KEY_P
        jnz update_open_pause
        mov eax, ebx
        and eax, KEY_LEFT | KEY_RIGHT
        cmp eax, KEY_LEFT
        je update_aim_repeat
        cmp eax, KEY_RIGHT
        je update_aim_repeat
        mov heldAim, 0                // No direction while both or neither are held.
        jmp update_fire
    update_aim_repeat:
        mov ecx, now
        cmp eax, heldAim
        jne update_aim_step          // Start a new direction with one immediate step.
        mov edx, ecx
        sub edx, lastAimTick         // Unsigned elapsed time handles tick wraparound.
        cmp edx, AIM_REPEAT_MS
        jb update_fire
    update_aim_step:
        mov heldAim, eax
        mov lastAimTick, ecx          // One repeat at most, even after a stalled frame.
        cmp eax, KEY_LEFT
        je update_aim_left
        push 1
        jmp update_aim
    update_aim_left:
        push -1
    update_aim:
        call Shooter_Aim
        add esp, 4
    update_fire:
        test esi, KEY_SPACE
        jz update_move
        call Shooter_Fire
    update_move:
        cmp shotActive, 0
        je update_done
        mov edi, score
        call Shooter_Update
        cmp shotActive, 0
        jne update_done
        cmp lastRow, 0
        jl update_lost                // No legal empty cell existed in the path.
        call Grid_CheckDanger
        test eax, eax
        jnz update_lost
        xor eax, eax
        cmp score, edi               // Match_Resolve owns both point awards.
        je update_drop
        inc eax                       // A changed score means a match popped.
    update_drop:
        push eax
        call Score_UpdateDrop
        add esp, 4
        call Grid_CheckDanger
        test eax, eax
        jnz update_lost
        call Grid_IsEmpty
        test eax, eax
        jz update_done
        mov edx, STATE_WIN
        jmp update_result

    update_open_pause:
        mov heldAim, 0
        mov gameState, STATE_PAUSED
        jmp update_done
    update_lost:
        mov edx, STATE_GAMEOVER
    update_result:
        mov eax, score
        cmp eax, highScore
        jle update_show_result
        mov highScore, eax
    update_show_result:
        mov gameState, edx
        jmp update_done
    update_to_menu:
        mov shotActive, 0
        mov gameState, STATE_MENU
        jmp update_done
    update_exit:
        mov gameState, STATE_EXIT
    update_done:
    }
}
