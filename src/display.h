#ifndef DISPLAY_H
#define DISPLAY_H

#include "heap.h"

/* ── ANSI color helpers ── */
#define CLR_RESET   "\033[0m"
#define CLR_BOLD    "\033[1m"
#define CLR_DIM     "\033[2m"
#define CLR_RED     "\033[31m"
#define CLR_GREEN   "\033[32m"
#define CLR_YELLOW  "\033[33m"
#define CLR_BLUE    "\033[34m"
#define CLR_MAGENTA "\033[35m"
#define CLR_CYAN    "\033[36m"
#define CLR_WHITE   "\033[37m"
#define CLR_BG_GREEN  "\033[42m"
#define CLR_BG_RED    "\033[41m"

/* ── Logging macros ── */
#define LOG_ALLOC(ptr, size) \
    printf(CLR_GREEN CLR_BOLD "  [ALLOC]" CLR_RESET \
           CLR_GREEN " %zu bytes at %p" CLR_RESET "\n", (size_t)(size), (void *)(ptr))

#define LOG_ALLOC_REUSE(ptr, size) \
    printf(CLR_GREEN CLR_BOLD "  [ALLOC]" CLR_RESET \
           CLR_GREEN " %zu bytes at %p" CLR_RESET \
           CLR_CYAN CLR_BOLD " (REUSED)" CLR_RESET "\n", (size_t)(size), (void *)(ptr))

#define LOG_FREE(ptr, size) \
    printf(CLR_YELLOW CLR_BOLD "  [FREE] " CLR_RESET \
           CLR_YELLOW "%zu bytes at %p" CLR_RESET "\n", (size_t)(size), (void *)(ptr))

#define LOG_ERROR(msg) \
    printf(CLR_RED CLR_BOLD "  [ERROR] " CLR_RESET CLR_RED msg CLR_RESET "\n")

#define LOG_INFO(msg) \
    printf(CLR_CYAN CLR_BOLD "  [INFO]  " CLR_RESET CLR_CYAN msg CLR_RESET "\n")

/* ── Display functions ── */
void chunk_list_dump(const char *label, const char *color, const Heap_Block_List *list);
void heap_visualize(void);
void print_banner(void);

#endif /* DISPLAY_H */
