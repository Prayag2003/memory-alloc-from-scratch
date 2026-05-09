#include <stdio.h>
#include <string.h>
#include "display.h"

#define VIS_WIDTH 72

/*
 * Prints a banner header for the program.
 */
void print_banner(void)
{
    printf("\n%s%s  ╔══════════════════════════════════════════════════════╗%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
    printf("%s%s  ║       Memory Allocator — From Scratch                ║%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
    printf("%s%s  ╚══════════════════════════════════════════════════════╝%s\n\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
}

/*
 * Prints every block in a block list as a
 * formatted table with index, address, size, and offset.
 */
void chunk_list_dump(const char *label, const char *color, const Heap_Block_List *list)
{
    printf("\n  %s%s%s (%zu blocks):\n", color, label, CLR_RESET, list->count);
    if (list->count == 0)
    {
        printf("    %s(empty)%s\n", CLR_DIM, CLR_RESET);
        return;
    }
    printf("    %s┌───────┬──────────────────┬────────────┐%s\n", CLR_DIM, CLR_RESET);
    printf("    %s│ Index │ Start Address    │ Size       │%s\n", CLR_DIM, CLR_RESET);
    printf("    %s├───────┼──────────────────┼────────────┤%s\n", CLR_DIM, CLR_RESET);
    for (size_t i = 0; i < list->count; i++)
    {
        size_t offset = (size_t)((char *)list->blocks[i].start - heap_buffer);
        printf("    %s│%s %s%-5zu%s %s│%s %p %s│%s %s%-10zu%s %s│%s",
               CLR_DIM, CLR_RESET,
               color, i, CLR_RESET,
               CLR_DIM, CLR_RESET,
               list->blocks[i].start,
               CLR_DIM, CLR_RESET,
               color, list->blocks[i].size, CLR_RESET,
               CLR_DIM, CLR_RESET);
        printf(" %s(offset: %zu)%s\n", CLR_DIM, offset, CLR_RESET);
    }
    printf("    %s└───────┴──────────────────┴────────────┘%s\n", CLR_DIM, CLR_RESET);
}

/* ── Heap Visualizer ──
 * Renders the heap as a horizontal bar.
 * '█' = allocated (green), '░' = free (dim),
 * scaled to fit a fixed terminal width.
 */
void heap_visualize(void)
{
    printf("\n  %s%s─── Heap Visualizer ──────────────────────────────────────────────────%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);

    /* Build a character map: 'A' = allocated, 'F' = free, '.' = unmapped */
    char bar[VIS_WIDTH];
    memset(bar, '.', VIS_WIDTH);

    size_t total_alloc = 0;
    size_t total_free = 0;

    /* Paint allocated blocks */
    for (size_t i = 0; i < heap_allocated_blocks.count; i++)
    {
        size_t offset = (size_t)((char *)heap_allocated_blocks.blocks[i].start - heap_buffer);
        size_t sz = heap_allocated_blocks.blocks[i].size;
        total_alloc += sz;
        size_t col_start = (offset * VIS_WIDTH) / HEAP_CAPACITY;
        size_t col_end = ((offset + sz) * VIS_WIDTH) / HEAP_CAPACITY;
        if (col_end == col_start && sz > 0)
            col_end = col_start + 1;
        for (size_t c = col_start; c < col_end && c < VIS_WIDTH; c++)
            bar[c] = 'A';
    }

    /* Paint free blocks */
    for (size_t i = 0; i < heap_freed_blocks.count; i++)
    {
        size_t offset = (size_t)((char *)heap_freed_blocks.blocks[i].start - heap_buffer);
        size_t sz = heap_freed_blocks.blocks[i].size;
        total_free += sz;
        size_t col_start = (offset * VIS_WIDTH) / HEAP_CAPACITY;
        size_t col_end = ((offset + sz) * VIS_WIDTH) / HEAP_CAPACITY;
        if (col_end == col_start && sz > 0)
            col_end = col_start + 1;
        for (size_t c = col_start; c < col_end && c < VIS_WIDTH; c++)
            if (bar[c] != 'A')
                bar[c] = 'F';
    }

    /* Render the bar */
    printf("  │");
    for (size_t c = 0; c < VIS_WIDTH; c++)
    {
        if (bar[c] == 'A')
            printf("%s%s█%s", CLR_BG_GREEN, CLR_WHITE, CLR_RESET);
        else if (bar[c] == 'F')
            printf("%s░%s", CLR_DIM, CLR_RESET);
        else
            printf("%s·%s", CLR_DIM, CLR_RESET);
    }
    printf("│\n");

    /* Legend */
    printf("  %s%s█%s = allocated   %s░%s = free\n",
           CLR_BG_GREEN, CLR_WHITE, CLR_RESET,
           CLR_DIM, CLR_RESET);

    /* Stats */
    double pct = (HEAP_CAPACITY > 0) ? (double)total_alloc / HEAP_CAPACITY * 100.0 : 0.0;
    printf("\n  %s%sHeap Stats:%s\n", CLR_BOLD, CLR_CYAN, CLR_RESET);
    printf("    Total capacity : %s%zu bytes%s\n", CLR_WHITE, (size_t)HEAP_CAPACITY, CLR_RESET);
    printf("    Allocated      : %s%s%zu bytes%s (%.1f%%)\n",
           CLR_BOLD, CLR_GREEN, total_alloc, CLR_RESET, pct);
    printf("    Free           : %s%zu bytes%s\n", CLR_YELLOW, total_free, CLR_RESET);
    printf("    Alloc blocks   : %s%zu%s\n", CLR_GREEN, heap_allocated_blocks.count, CLR_RESET);
    printf("    Free  blocks   : %s%zu%s\n", CLR_YELLOW, heap_freed_blocks.count, CLR_RESET);
    printf("  %s%s────────────────────────────────────────────────────────────────────────%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
}
