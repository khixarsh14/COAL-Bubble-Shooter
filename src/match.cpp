// Owner: Khizar Sheikh.
// Concepts: recursive four-neighbour flood-fill, cdecl stack frames, visited arrays.
#include "game.h"

enum { TOTAL_CELLS = COLS * ROWS };

void __cdecl Match_Fill(int col, int row, int color, int mode)
{
    // Mark one component without changing grid or score. Each recursive call
    // reloads parameters from its own compiler-managed frame; no live register
    // value is assumed to survive a child call. Never change EBP manually.
    __asm {
        mov ecx, col
        cmp ecx, 0
        jl match_fill_done
        cmp ecx, COLS
        jge match_fill_done
        mov eax, row
        cmp eax, 0
        jl match_fill_done
        cmp eax, ROWS
        jge match_fill_done
        shl eax, 4              // row * 16, after signed bounds checks.
        add eax, ecx            // index = row * COLS + col.
        lea edx, visited
        cmp byte ptr [edx + eax], 0
        jne match_fill_done     // Stop cycles and avoid counting a cell twice.
        lea edx, grid
        movzx ecx, byte ptr [edx + eax]
        cmp ecx, EMPTY
        je match_fill_done
        cmp mode, 1
        je match_fill_mark      // Connectivity accepts any nonempty color.
        cmp mode, 0
        jne match_fill_done     // Defensive: unknown modes leave scratch unchanged.
        cmp ecx, color          // Compare the whole int, not a truncated color byte.
        jne match_fill_done
    match_fill_mark:
        lea edx, visited
        mov byte ptr [edx + eax], 1 // Mark BEFORE recursing, to break cycles.
        inc popCount

        // UP: push mode, color, row-1, col (right-to-left cdecl arguments).
        push mode
        push color
        mov eax, row
        dec eax
        push eax
        push col
        call Match_Fill
        add esp, 16             // Four int arguments; CALL's return address is separate.

        // DOWN: reload parent parameters after the child has returned.
        push mode
        push color
        mov eax, row
        inc eax
        push eax
        push col
        call Match_Fill
        add esp, 16

        // LEFT
        push mode
        push color
        push row
        mov eax, col
        dec eax
        push eax
        call Match_Fill
        add esp, 16

        // RIGHT
        push mode
        push color
        push row
        mov eax, col
        inc eax
        push eax
        call Match_Fill
        add esp, 16
    match_fill_done:
    }
    // MSVC's normal epilogue restores the frame and returns; no hand-written RET.
}

void __cdecl Match_Resolve(int col, int row)
{
    int matchedCount;
    // Validate first, find a same-color component, pop only groups of 3+, and
    // award pop points BEFORE the floating search reuses visited and popCount.
    __asm {
        mov ecx, col
        cmp ecx, 0
        jl match_resolve_done
        cmp ecx, COLS
        jge match_resolve_done
        mov eax, row
        cmp eax, 0
        jl match_resolve_done
        cmp eax, ROWS
        jge match_resolve_done
        shl eax, 4
        add eax, ecx
        lea esi, grid
        movzx ebx, byte ptr [esi + eax] // Save selected color across scratch clearing.
        cmp ebx, EMPTY
        je match_resolve_done
        cld
        xor eax, eax
        lea edi, visited
        mov ecx, TOTAL_CELLS
        rep stosb
        mov popCount, 0
        push 0                  // mode 0: exact color.
        push ebx                // selected color
        push row
        push col
        call Match_Fill
        add esp, 16
        mov eax, popCount
        cmp eax, MIN_MATCH
        jl match_resolve_done   // No pop means NO floating removal or points.
        mov matchedCount, eax   // A frame-local copy survives later scratch reuse.
        lea esi, grid
        lea edi, visited
        mov ecx, TOTAL_CELLS
    match_resolve_pop:
        cmp byte ptr [edi], 0
        je match_resolve_next
        mov byte ptr [esi], EMPTY
    match_resolve_next:
        inc esi
        inc edi
        dec ecx
        jnz match_resolve_pop
        mov eax, matchedCount
        imul eax, POINTS_POP    // Matched bubble COUNT -> points, exactly once.
        add score, eax
        call Match_DropFloating // No arguments; this service owns floating points.
    match_resolve_done:
    }
}

void __cdecl Match_DropFloating()
{
    int topCol;
    // Caller invokes this only after a successful pop. Mark every top-connected
    // component in mode 1, then remove and score each unmarked nonempty cell.
    __asm {
        cld                     // Always establish direction before REP STOSB.
        xor eax, eax
        lea edi, visited
        mov ecx, TOTAL_CELLS
        rep stosb
        mov popCount, 0         // Now counts TOP-CONNECTED cells, not matched cells.
        mov topCol, 0          // A frame-local index survives recursive child calls.
    match_float_top:
        mov eax, topCol
        lea esi, grid
        cmp byte ptr [esi + eax], EMPTY
        je match_float_next_top
        push 1                  // mode 1: connectivity across all nonempty colors.
        push 0                  // color ignored in mode 1.
        push 0                  // row = 0.
        push eax                // col = topCol.
        call Match_Fill
        add esp, 16
    match_float_next_top:
        inc topCol              // Reloaded from the frame; no volatile register reliance.
        cmp topCol, COLS
        jl match_float_top
        lea esi, grid
        lea edi, visited
        mov ecx, TOTAL_CELLS
    match_float_scan:
        cmp byte ptr [esi], EMPTY
        je match_float_next
        cmp byte ptr [edi], 0
        jne match_float_next    // Any path to row 0 keeps this bubble attached.
        mov byte ptr [esi], EMPTY
        add score, POINTS_DROP // Score each removed floating bubble exactly once.
    match_float_next:
        inc esi
        inc edi
        dec ecx
        jnz match_float_scan
    }
}
