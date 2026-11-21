// Concepts: indexed aim, signed fixed-point movement, and cdecl collision queries.
#include "console.h"
#include <stddef.h>

enum { WALL = COLS * SUBPIX, STEP_COUNT = 4,
    // The smallest upward substep in the aim table is 32 fixed-point units.
    GUIDE_STEPS = (LAUNCH_ROW + 1) * SUBPIX / 32 };
enum { FLIGHT, LANDED, BLOCKED };
struct ShotMotion { int x, y, dx, dy, col, row; };
enum {
    MOTION_X = offsetof(ShotMotion, x), MOTION_Y = offsetof(ShotMotion, y),
    MOTION_DX = offsetof(ShotMotion, dx), MOTION_DY = offsetof(ShotMotion, dy),
    MOTION_COL = offsetof(ShotMotion, col), MOTION_ROW = offsetof(ShotMotion, row)
};

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

static void __cdecl StartMotion(ShotMotion* motion, int index)
{
    // Both prediction and firing initialize the same fixed-point state.
    __asm {
        mov ecx, motion
        mov eax, index
        mov edx, angleDX[eax * 4]
        mov [ecx + MOTION_DX], edx
        mov edx, angleDY[eax * 4]
        neg edx
        mov [ecx + MOTION_DY], edx
        mov dword ptr [ecx + MOTION_X], LAUNCH_COL * SUBPIX + SUBPIX / 2
        mov dword ptr [ecx + MOTION_Y], LAUNCH_ROW * SUBPIX + SUBPIX / 2
        mov dword ptr [ecx + MOTION_COL], -1
        mov dword ptr [ecx + MOTION_ROW], -1
    }
}

static void __cdecl SaveMotion(const ShotMotion* motion)
{
    __asm {
        mov ecx, motion
        mov eax, [ecx + MOTION_X]
        mov shotX, eax
        mov eax, [ecx + MOTION_Y]
        mov shotY, eax
        mov eax, [ecx + MOTION_DX]
        mov shotDX, eax
        mov eax, [ecx + MOTION_DY]
        mov shotDY, eax
        mov eax, [ecx + MOTION_COL]
        mov lastCol, eax
        mov eax, [ecx + MOTION_ROW]
        mov lastRow, eax
    }
}

void __cdecl Shooter_Fire()
{
    ShotMotion motion;
    // Start and publish a live shot only when the launcher is idle.
    __asm {
        cmp shotActive, 0
        jne fire_done
        lea esi, motion
        push aimIndex
        push esi
        call StartMotion
        add esp, 8
        push esi
        call SaveMotion
        add esp, 4
        mov shotActive, 1
    fire_done:
    }
}

static int __cdecl AdvanceMotion(ShotMotion* motion, int steps, BYTE* guide)
{
    int fromX, fromY, endCol, endRow, result;
    // Visit each crossed cell in order. EBX holds the private motion state.
    // See docs/OUT_OF_OUTLINE.md, Projectile grid crossings.
    __asm {
        mov ebx, motion
    shot_step:
        mov eax, dword ptr [ebx + MOTION_DX]
        sar eax, 2
        mov edx, dword ptr [ebx + MOTION_X]
        mov fromX, edx
        add eax, dword ptr [ebx + MOTION_X]
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
        neg dword ptr [ebx + MOTION_DX]
    shot_store_x:
        mov dword ptr [ebx + MOTION_X], eax
        sar eax, 8
        mov esi, eax                // ESI = column across cdecl calls.
        mov endCol, eax
        mov eax, dword ptr [ebx + MOTION_DY]
        sar eax, 2
        mov edx, dword ptr [ebx + MOTION_Y]
        mov fromY, edx
        add dword ptr [ebx + MOTION_Y], eax
        mov eax, dword ptr [ebx + MOTION_Y]
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
        cmp dword ptr [ebx + MOTION_DX], 0
        jl shot_x_distance
        neg eax
        add eax, SUBPIX
    shot_x_distance:
        mov edx, dword ptr [ebx + MOTION_DY]
        sar edx, 2
        neg edx
        imul eax, edx
        mov ecx, fromY
        and ecx, SUBPIX - 1
        mov edx, dword ptr [ebx + MOTION_DX]
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
        mov [ebx + MOTION_COL], esi
        mov [ebx + MOTION_ROW], edi
        mov edx, guide
        test edx, edx
        jz shot_check_ceiling
        mov eax, edi
        shl eax, 4
        add eax, esi
        mov byte ptr [edx + eax], GUIDE_DOT
    shot_check_ceiling:
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
        dec steps
        jnz shot_step
        mov result, FLIGHT
        jmp shot_done

    shot_use_previous:
        mov edi, [ebx + MOTION_ROW]
        mov esi, [ebx + MOTION_COL]
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
        mov result, LANDED
        jmp shot_done
    shot_blocked:
        mov dword ptr [ebx + MOTION_COL], -1
        mov dword ptr [ebx + MOTION_ROW], -1
        mov result, BLOCKED
    shot_done:
    }
    return result;
}

void __cdecl Shooter_Update()
{
    ShotMotion motion;
    // Only the live caller commits a placement or advances the color queue.
    __asm {
        cmp shotActive, 0
        je update_done
        lea esi, motion
        mov eax, shotX
        mov [esi + MOTION_X], eax
        mov eax, shotY
        mov [esi + MOTION_Y], eax
        mov eax, shotDX
        mov [esi + MOTION_DX], eax
        mov eax, shotDY
        mov [esi + MOTION_DY], eax
        mov eax, lastCol
        mov [esi + MOTION_COL], eax
        mov eax, lastRow
        mov [esi + MOTION_ROW], eax
        push 0
        push STEP_COUNT
        push esi
        call AdvanceMotion
        add esp, 12
        mov ebx, eax                 // Save the flight result across the copy.
        push esi
        call SaveMotion
        add esp, 4
        cmp ebx, FLIGHT
        je update_done
        mov shotActive, 0
        cmp ebx, LANDED
        jne update_done
        mov esi, lastCol
        mov edi, lastRow
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
    update_done:
    }
}

void __cdecl Shooter_Predict(int index, BYTE* guide)
{
    ShotMotion motion;
    // The guide is a caller-owned ROWS*COLS mask, independent of the game grid.
    __asm {
        cld
        mov edi, guide
        xor eax, eax
        mov ecx, COLS * ROWS
        rep stosb
        mov eax, index
        cmp eax, NUM_ANGLES
        jae predict_done
        lea esi, motion
        push eax
        push esi
        call StartMotion
        add esp, 8
        push guide
        push GUIDE_STEPS
        push esi
        call AdvanceMotion
        add esp, 12
        cmp eax, LANDED
        jne predict_done
        mov eax, [esi + MOTION_ROW]
        shl eax, 4
        add eax, [esi + MOTION_COL]
        mov edi, guide
        mov byte ptr [edi + eax], GUIDE_LANDING
    predict_done:
    }
}
