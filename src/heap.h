#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

#define HEAP_CAPACITY 128000 // 128 KB
#define HEAP_BLOCK_LIST_CAPACITY 8192 // 8 KB

typedef struct
{
    void *start;
    size_t size;
} MemBlock;

typedef struct
{
    MemBlock blocks[HEAP_BLOCK_LIST_CAPACITY];
    size_t count;
} MemBlockList;

/* ── Global state (defined in main.c) ── */
extern char heap_buffer[HEAP_CAPACITY];
extern MemBlockList allocated_blocks;
extern MemBlockList free_blocks;

/* ── Allocator API ── */
void *heap_alloc(size_t size);
void  heap_free(void *ptr);
void  heap_coalesce_free_blocks(void);

/* ── Block list operations ── */
void block_list_add(MemBlockList *list, void *start, size_t size);
int  block_list_find(const MemBlockList *list, void *ptr);
void block_list_remove(MemBlockList *list, size_t index);
int  block_compare(const void *a, const void *b);

#endif /* HEAP_H */
