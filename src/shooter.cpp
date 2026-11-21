// Concepts: indexed aim, signed fixed-point movement, and cdecl collision queries.
#include "console.h"

enum { WALL = COLS * SUBPIX, STEP_COUNT = 4 };

void __cdecl Shooter_Aim(int step)
{
    // Clamp the requested index. JO handles a large positive step wrapping.
    __asm {
        mov eax, aimIndex
        add eax, step
        jo aim_high
        cmp eax, 0
        jl aim_low
        cmp eax, NUM_ANGLES - 1
        jle aim_store
    aim_high:
        mov eax, NUM_ANGLES - 1
        jmp aim_store
    aim_low:
        xor eax, eax
    aim_store:
        mov aimIndex, eax
    }
}

void __cdecl Shooter_Fire()
{
    // Launch from the center of column 7, one row below the board.
    __asm {
        cmp shotActive, 0
        jne fire_done
        mov eax, aimIndex
        mov edx, angleDX[eax * 4]
        mov shotDX, edx
        mov edx, angleDY[eax * 4]
        neg edx                     // Screen Y increases downward.
        mov shotDY, edx
        mov shotX, LAUNCH_COL * SUBPIX + SUBPIX / 2
        mov shotY, LAUNCH_ROW * SUBPIX + SUBPIX / 2
        mov lastCol, -1
        mov lastRow, -1
        mov shotActive, 1
    fire_done:
    }
}

void __cdecl Shooter_Update()
{
    int fromX, fromY, endCol, endRow;
    // Visit each crossed cell in order. EBX counts remaining substeps.
    // See docs/OUT_OF_OUTLINE.md, Projectile grid crossings.
    __asm {
        cmp shotActive, 0
        je shot_done
        mov ebx, STEP_COUNT
    shot_step:
        mov eax, shotDX
        sar eax, 2
        mov edx, shotX
        mov fromX, edx
        add eax, shotX
        cmp eax, 0
        jl shot_left_wall
        cmp eax, WALL
        jl shot_store_x
        mov edx, 2 * WALL - 1       // Mirror overshoot into [0, WALL-1].
        sub edx, eax
        mov eax, edx
        jmp shot_reflect
    shot_left_wall:
        not eax                     // -x-1 mirrors the left boundary.
    shot_reflect:
        neg shotDX
    shot_store_x:
        mov shotX, eax
        sar eax, 8
        mov esi, eax                // ESI = column across cdecl calls.
        mov endCol, eax
        mov eax, shotDY
        sar eax, 2
        mov edx, shotY
        mov fromY, edx
        add shotY, eax
        mov eax, shotY
        sar eax, 8
        cmp eax, ROWS
        jge shot_continue           // Still in the launcher area.
        cmp eax, 0
        jge shot_row_ready
        xor eax, eax
    shot_row_ready:
        mov edi, eax                // EDI = row across cdecl calls.
        mov endRow, eax
        mov ecx, fromX
        sar ecx, 8
        cmp ecx, esi
        je shot_visit
        mov edx, fromY
        sar edx, 8
        cmp edx, edi
        je shot_visit

        // Both axes crossed a boundary. Compare the crossing times by
        // multiplying each distance by the other axis's speed.
        mov eax, fromX
        and eax, SUBPIX - 1
        cmp shotDX, 0
        jl shot_x_distance
        neg eax
        add eax, SUBPIX
    shot_x_distance:
        mov edx, shotDY
        sar edx, 2
        neg edx
        imul eax, edx
        mov ecx, fromY
        and ecx, SUBPIX - 1
        mov edx, shotDX
        sar edx, 2
        test edx, edx
        jns shot_x_speed
        neg edx
    shot_x_speed:
        imul ecx, edx
        cmp eax, ecx
        je shot_visit               // Equal times touch a corner only.
        jb shot_x_first
        mov esi, fromX              // Y crossed first: visit the old column.
        sar esi, 8
        jmp shot_visit
    shot_x_first:
        mov edi, fromY              // X crossed first: visit the old row.
        sar edi, 8
        cmp edi, ROWS
        jae shot_end_cell           // The intermediate cell is in the launch lane.
    shot_visit:
        push edi
        push esi
        call Grid_Get
        add esp, 8
        test eax, eax
        jnz shot_use_previous
        mov lastCol, esi
        mov lastRow, edi
        test edi, edi
        jz shot_place               // Entered an empty ceiling cell.
        cmp esi, endCol
        jne shot_end_cell
        cmp edi, endRow
        je shot_continue
    shot_end_cell:
        mov esi, endCol
        mov edi, endRow
        jmp shot_visit
    shot_continue:
        dec ebx
        jnz shot_step
        jmp shot_done

    shot_use_previous:
        mov edi, lastRow
        mov esi, lastCol
        cmp edi, ROWS               // Unsigned checks reject -1 as well.
        jae shot_blocked
        cmp esi, COLS
        jae shot_blocked
        push edi
        push esi
        call Grid_Get
        add esp, 8
        test eax, eax
        jnz shot_blocked
    shot_place:
        push curColor
        push edi
        push esi
        call Grid_Set
        add esp, 12
        push edi
        push esi
        call Match_Resolve
        add esp, 8
        mov eax, nextColor
        mov curColor, eax
        push NUM_COLORS
        call Con_Rand
        add esp, 4
        inc eax
        mov nextColor, eax
        mov shotActive, 0
        jmp shot_done
    shot_blocked:
        // No empty cell was traversed. Report a failed landing to Game_Update.
        mov lastRow, -1
        mov shotActive, 0
    shot_done:
    }
}
