#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "display.h"

#define VIS_WIDTH 72

int g_logging_enabled = 1;

void print_banner(void)
{
    printf("\n%s%s  ╔══════════════════════════════════════════════════════╗%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
    printf("%s%s  ║       Memory Allocator — From Scratch                ║%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
    printf("%s%s  ╚══════════════════════════════════════════════════════╝%s\n\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
}

void block_list_dump(const char *label, const char *color, const MemBlockList *list)
{
    printf("\n  %s%s%s (%zu blocks):\n", color, label, CLR_RESET, list->count);
    if (list->count == 0)
    {
        printf("    %s(empty)%s\n", CLR_DIM, CLR_RESET);
        return;
    }
    printf("    %s┌───────┬──────────────────┬────────────┬──────────────┐%s\n", CLR_DIM, CLR_RESET);
    printf("    %s│ Index │ Start Address    │ Size       │ Offset       │%s\n", CLR_DIM, CLR_RESET);
    printf("    %s├───────┼──────────────────┼────────────┼──────────────┤%s\n", CLR_DIM, CLR_RESET);
    for (size_t i = 0; i < list->count; i++)
    {
        size_t offset = (size_t)((char *)list->blocks[i].start - heap_buffer);
        printf("    %s│%s %s%-5zu%s %s│%s %-16p %s│%s %s%-10zu%s %s│%s %s%-12zu%s %s│%s\n",
               CLR_DIM, CLR_RESET,
               color, i, CLR_RESET,
               CLR_DIM, CLR_RESET,
               list->blocks[i].start,
               CLR_DIM, CLR_RESET,
               color, list->blocks[i].size, CLR_RESET,
               CLR_DIM, CLR_RESET,
               CLR_DIM, offset, CLR_RESET,
               CLR_DIM, CLR_RESET);
    }
    printf("    %s└───────┴──────────────────┴────────────┴──────────────┘%s\n", CLR_DIM, CLR_RESET);
}

/* Segment: a merged view of one allocated or free block */
typedef struct {
    size_t offset;
    size_t size;
    char   type; /* 'A' = allocated, 'F' = free */
} Segment;

static int seg_cmp(const void *a, const void *b)
{
    const Segment *sa = (const Segment *)a;
    const Segment *sb = (const Segment *)b;
    if (sa->offset < sb->offset) return -1;
    if (sa->offset > sb->offset) return  1;
    return 0;
}

/*
 * Advanced Heap Visualizer
 *
 * Layout (fixed 16 lines from this function):
 *   1  (blank)
 *   2  ─── Heap Visualizer ───
 *   3  Allocated[N] ┃ sizes ┃
 *   4  Free[N]      ┃ sizes ┃
 *   5  (blank)
 *   6  ┌──────┬──────┬──────┐   segmented top border
 *   7  │A:10██│░F:5░░│A:30██│   labeled segments
 *   8  └──────┴──────┴──────┘   segmented bottom border
 *   9   0                125K   offset ruler
 *  10  legend
 *  11  (blank)
 *  12  Usage [━━━━━━] X.X%
 *  13  stats line
 *  14  ────────────────────
 */
void heap_visualize(void)
{
    /* Line 1-2: Header */
    printf("\n  %s%s─── Heap Visualizer ──────────────────────────────────────────────────%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);

    /* Build merged segment list */
    Segment segs[2048];
    size_t seg_count = 0;
    size_t total_alloc = 0, total_free = 0;

    for (size_t i = 0; i < allocated_blocks.count; i++) {
        segs[seg_count].offset = (size_t)((char *)allocated_blocks.blocks[i].start - heap_buffer);
        segs[seg_count].size   = allocated_blocks.blocks[i].size;
        segs[seg_count].type   = 'A';
        total_alloc += segs[seg_count].size;
        seg_count++;
    }
    for (size_t i = 0; i < free_blocks.count; i++) {
        segs[seg_count].offset = (size_t)((char *)free_blocks.blocks[i].start - heap_buffer);
        segs[seg_count].size   = free_blocks.blocks[i].size;
        segs[seg_count].type   = 'F';
        total_free += segs[seg_count].size;
        seg_count++;
    }

    qsort(segs, seg_count, sizeof(Segment), seg_cmp);

    /* Calculate proportional column widths */
    int widths[2048];
    int total_cols = 0;
    for (size_t i = 0; i < seg_count; i++) {
        widths[i] = (int)((segs[i].size * (size_t)VIS_WIDTH) / HEAP_CAPACITY);
        if (widths[i] < 1) widths[i] = 1;
        total_cols += widths[i];
    }
    /* Adjust last segment so columns sum to exactly VIS_WIDTH */
    if (seg_count > 0 && total_cols != VIS_WIDTH) {
        widths[seg_count - 1] += VIS_WIDTH - total_cols;
        if (widths[seg_count - 1] < 1) widths[seg_count - 1] = 1;
    }

    /* Line 3: Allocated Array boxes */
    printf("  %s%sAllocated[%zu]%s ", CLR_BOLD, CLR_GREEN, allocated_blocks.count, CLR_RESET);
    if (allocated_blocks.count == 0) {
        printf("%s(empty)%s", CLR_DIM, CLR_RESET);
    } else {
        for (size_t i = 0; i < allocated_blocks.count && i < 10; i++) {
            size_t sz = allocated_blocks.blocks[i].size;
            if (sz > 9999) printf("%s┃%s%s%3zuK%s", CLR_DIM, CLR_RESET, CLR_GREEN, sz/1024, CLR_RESET);
            else           printf("%s┃%s%s%4zu%s", CLR_DIM, CLR_RESET, CLR_GREEN, sz, CLR_RESET);
        }
        printf("%s┃%s", CLR_DIM, CLR_RESET);
        if (allocated_blocks.count > 10) printf(" %s+%zu more%s", CLR_DIM, allocated_blocks.count - 10, CLR_RESET);
    }
    printf("\n");

    /* Line 4: Free Array boxes */
    printf("  %s%sFree[%zu]%s      ", CLR_BOLD, CLR_YELLOW, free_blocks.count, CLR_RESET);
    if (free_blocks.count == 0) {
        printf("%s(empty)%s", CLR_DIM, CLR_RESET);
    } else {
        for (size_t i = 0; i < free_blocks.count && i < 10; i++) {
            size_t sz = free_blocks.blocks[i].size;
            if (sz > 9999) printf("%s┃%s%s%3zuK%s", CLR_DIM, CLR_RESET, CLR_YELLOW, sz/1024, CLR_RESET);
            else           printf("%s┃%s%s%4zu%s", CLR_DIM, CLR_RESET, CLR_YELLOW, sz, CLR_RESET);
        }
        printf("%s┃%s", CLR_DIM, CLR_RESET);
        if (free_blocks.count > 10) printf(" %s+%zu more%s", CLR_DIM, free_blocks.count - 10, CLR_RESET);
    }
    printf("\n");

    /* Line 5-6: blank + top border of segmented map */
    printf("\n  %s┌%s", CLR_DIM, CLR_RESET);
    for (size_t i = 0; i < seg_count; i++) {
        printf("%s", CLR_DIM);
        for (int j = 0; j < widths[i]; j++) printf("─");
        printf("%s", CLR_RESET);
        if (i < seg_count - 1) printf("%s┬%s", CLR_DIM, CLR_RESET);
    }
    printf("%s┐%s\n", CLR_DIM, CLR_RESET);

    /* Line 7: Content row — labeled, colored segments */
    printf("  %s│%s", CLR_DIM, CLR_RESET);
    for (size_t i = 0; i < seg_count; i++) {
        int w = widths[i];

        /* Build a label like "A:30" or "F:125K" */
        char label[32];
        if (segs[i].size >= 10000)
            snprintf(label, sizeof(label), "%c:%zuK", segs[i].type, segs[i].size / 1024);
        else
            snprintf(label, sizeof(label), "%c:%zu", segs[i].type, segs[i].size);

        int lbl_len = (int)strlen(label);
        int show_label = (lbl_len <= w);
        int pad_left  = show_label ? (w - lbl_len) / 2 : 0;
        int pad_right = show_label ? (w - lbl_len - pad_left) : 0;

        if (segs[i].type == 'A') {
            /* Green background, white text */
            printf("%s%s", CLR_BG_GREEN, CLR_WHITE);
            if (show_label) {
                for (int j = 0; j < pad_left; j++)  printf("█");
                printf("%s", label);
                for (int j = 0; j < pad_right; j++) printf("█");
            } else {
                for (int j = 0; j < w; j++) printf("█");
            }
            printf("%s", CLR_RESET);
        } else {
            /* Free: dim fill, yellow label */
            if (show_label) {
                printf("%s", CLR_DIM);
                for (int j = 0; j < pad_left; j++)  printf("░");
                printf("%s%s%s", CLR_RESET, CLR_YELLOW, label);
                printf("%s%s", CLR_RESET, CLR_DIM);
                for (int j = 0; j < pad_right; j++) printf("░");
                printf("%s", CLR_RESET);
            } else {
                printf("%s", CLR_DIM);
                for (int j = 0; j < w; j++) printf("░");
                printf("%s", CLR_RESET);
            }
        }
        printf("%s│%s", CLR_DIM, CLR_RESET);
    }
    printf("\n");

    /* Line 8: bottom border */
    printf("  %s└%s", CLR_DIM, CLR_RESET);
    for (size_t i = 0; i < seg_count; i++) {
        printf("%s", CLR_DIM);
        for (int j = 0; j < widths[i]; j++) printf("─");
        printf("%s", CLR_RESET);
        if (i < seg_count - 1) printf("%s┴%s", CLR_DIM, CLR_RESET);
    }
    printf("%s┘%s\n", CLR_DIM, CLR_RESET);

    /* Line 9: offset ruler */
    printf("  %s 0", CLR_DIM);
    /* Calculate total rendered width to right-align the capacity label */
    int rendered_width = 0;
    for (size_t i = 0; i < seg_count; i++) rendered_width += widths[i];
    rendered_width += (int)seg_count - 1; /* internal separators */
    int ruler_pad = rendered_width - 5; /* "0" already printed, leave room for "125K" */
    if (ruler_pad < 1) ruler_pad = 1;
    for (int i = 0; i < ruler_pad; i++) printf(" ");
    printf("%zuK%s\n", (size_t)HEAP_CAPACITY / 1024, CLR_RESET);

    /* Line 10: legend */
    printf("  %s%s█%s = alloc (%sA%s → heap)   %s░%s = free (%sF%s → heap)\n",
           CLR_BG_GREEN, CLR_WHITE, CLR_RESET,
           CLR_GREEN, CLR_RESET,
           CLR_DIM, CLR_RESET,
           CLR_YELLOW, CLR_RESET);

    /* Line 11-12: blank + usage progress bar */
    double pct = (HEAP_CAPACITY > 0) ? (double)total_alloc / HEAP_CAPACITY * 100.0 : 0.0;
    int filled = (int)(pct * 40.0 / 100.0);
    if (filled > 40) filled = 40;
    printf("\n  %sUsage%s [", CLR_BOLD, CLR_RESET);
    for (int i = 0; i < 40; i++) {
        if (i < filled) printf("%s━%s", CLR_GREEN, CLR_RESET);
        else            printf("%s━%s", CLR_DIM, CLR_RESET);
    }
    printf("] %s%.1f%%%s\n", CLR_GREEN, pct, CLR_RESET);

    /* Line 13: compact stats */
    size_t largest_free = 0;
    for (size_t i = 0; i < free_blocks.count; i++) {
        if (free_blocks.blocks[i].size > largest_free)
            largest_free = free_blocks.blocks[i].size;
    }
    double frag = (free_blocks.count > 1 && total_free > 0)
        ? (double)(total_free - largest_free) / total_free * 100.0 : 0.0;

    printf("  %sAlloc%s: %s%zu B%s (%zu blks)  "
           "%sFree%s: %s%zu B%s (%zu blks)  "
           "%sFrag%s: %s%.1f%%%s\n",
           CLR_GREEN, CLR_RESET, CLR_GREEN, total_alloc, CLR_RESET, allocated_blocks.count,
           CLR_YELLOW, CLR_RESET, CLR_YELLOW, total_free, CLR_RESET, free_blocks.count,
           frag > 10.0 ? CLR_RED : CLR_YELLOW, CLR_RESET,
           frag > 10.0 ? CLR_RED : CLR_YELLOW, frag, CLR_RESET);

    /* Line 14: footer */
    printf("  %s%s────────────────────────────────────────────────────────────────────────%s\n",
           CLR_BOLD, CLR_MAGENTA, CLR_RESET);
}
