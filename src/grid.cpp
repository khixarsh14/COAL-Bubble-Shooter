// Owner: Khizar Sheikh.
// Concepts: row-major indexing, bounds checks, loops, cdecl calls, string copying.
#include "game.h"
#include "console.h"

// Local assembly constants; the shared interface and coordinate order stay unchanged.
enum { TOTAL_CELLS = COLS * ROWS, SHIFT_CELLS = COLS * (ROWS - 1) };

void __cdecl Grid_Init()
{
    // Clear 208 bytes, then fill min(4 + level, 8) rows for valid levels (>=1).
    __asm {
        cld                     // String writes must advance toward higher addresses.
        xor eax, eax            // AL = EMPTY for byte stores.
        lea edi, grid
        mov ecx, TOTAL_CELLS
        rep stosb               // Write AL to every grid byte.
        mov ebx, level
        cmp ebx, 1
        jge grid_init_valid_level
        mov ebx, 1              // Defensive fallback for a nonpositive level.
    grid_init_valid_level:
        cmp ebx, 4
        jle grid_init_bounded_level
        mov ebx, 4              // Cap before adding, so even INT_MAX cannot overflow.
    grid_init_bounded_level:
        add ebx, 4              // Levels 1,2,3,4+ start with 5,6,7,8 rows.
        shl ebx, 4              // Multiply row count by 16 columns.
        lea edi, grid
    grid_init_fill:
        push NUM_COLORS         // cdecl argument: Con_Rand(5) returns 0..4 in EAX.
        call Con_Rand           // EBX and EDI survive this normal cdecl call.
        add esp, 4              // Caller removes the one 4-byte argument.
        inc eax                 // Convert 0..4 into bubble colors 1..5.
        mov byte ptr [edi], al
        inc edi
        dec ebx
        jnz grid_init_fill
    }
}

int __cdecl Grid_Get(int col, int row)
{
    int result;
    // Validate signed coordinates before forming an address; invalid reads return EMPTY.
    __asm {
        mov ecx, col
        cmp ecx, 0
        jl grid_get_invalid
        cmp ecx, COLS
        jge grid_get_invalid
        mov eax, row
        cmp eax, 0
        jl grid_get_invalid
        cmp eax, ROWS
        jge grid_get_invalid
        shl eax, 4              // row * 16, because COLS is 16.
        add eax, ecx            // Linear index = row * COLS + col.
        lea edx, grid
        movzx eax, byte ptr [edx + eax] // Expand the unsigned byte into an int.
        jmp grid_get_done
    grid_get_invalid:
        xor eax, eax
    grid_get_done:
        mov result, eax
    }
    return result;
}

void __cdecl Grid_Set(int col, int row, int color)
{
    // Ignore invalid coordinates or colors; EMPTY (0) is a valid erase value.
    __asm {
        mov edx, col
        cmp edx, 0
        jl grid_set_done
        cmp edx, COLS
        jge grid_set_done
        mov eax, row
        cmp eax, 0
        jl grid_set_done
        cmp eax, ROWS
        jge grid_set_done
        mov ecx, color
        cmp ecx, EMPTY
        jl grid_set_done
        cmp ecx, NUM_COLORS
        jg grid_set_done
        shl eax, 4
        add eax, edx
        lea esi, grid
        mov byte ptr [esi + eax], cl // Only the checked color byte is stored.
    grid_set_done:
    }
}

void __cdecl Grid_ShiftDown()
{
    // Copy old rows 0..11 to rows 1..12 backward; discard old row 12 and clear row 0.
    __asm {
        lea esi, grid
        add esi, SHIFT_CELLS - 1 // Last source byte: index 191 (row 11, col 15).
        lea edi, grid
        add edi, TOTAL_CELLS - 1 // Last destination byte: index 207.
        mov ecx, SHIFT_CELLS     // Exactly 192 overlapping bytes.
        std                     // Copy from high addresses toward low addresses.
        rep movsb
        cld                     // Restore forward direction immediately, before any call.
        xor eax, eax
        lea edi, grid
        mov ecx, COLS
        rep stosb               // New row 0 is EMPTY until Grid_AddTopRow is called.
    }
}

void __cdecl Grid_AddTopRow()
{
    // Fill exactly row 0; preserve all rows below it.
    __asm {
        lea edi, grid
        mov ebx, COLS
    grid_top_fill:
        push NUM_COLORS
        call Con_Rand           // The persistent pointer/count use cdecl-preserved EDI/EBX.
        add esp, 4
        inc eax
        mov byte ptr [edi], al
        inc edi
        dec ebx
        jnz grid_top_fill
    }
}

int __cdecl Grid_CheckDanger()
{
    int result;
    // Scan the final two rows (32 cells), including row 12's final byte.
    __asm {
        lea esi, grid
        add esi, DANGER_ROW * COLS
        mov ecx, TOTAL_CELLS - DANGER_ROW * COLS
        xor eax, eax            // Default: no bubble in either danger row.
    grid_danger_scan:
        cmp byte ptr [esi], EMPTY
        jne grid_danger_found
        inc esi
        dec ecx
        jnz grid_danger_scan
        jmp grid_danger_done
    grid_danger_found:
        mov eax, 1
    grid_danger_done:
        mov result, eax
    }
    return result;
}

int __cdecl Grid_IsEmpty()
{
    int result;
    // Return 1 only if every byte is EMPTY; any nonzero byte means not empty.
    __asm {
        lea esi, grid
        mov ecx, TOTAL_CELLS
        mov eax, 1
    grid_empty_scan:
        cmp byte ptr [esi], EMPTY
        jne grid_empty_found
        inc esi
        dec ecx
        jnz grid_empty_scan
        jmp grid_empty_done
    grid_empty_found:
        xor eax, eax
    grid_empty_done:
        mov result, eax
    }
    return result;
}
