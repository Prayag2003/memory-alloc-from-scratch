#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>

#define HEAP_CAPACITY 128000 // 128 KB
#define HEAP_BLOCK_LIST_CAPACITY 8192 // 8 KB

typedef struct
{
    void *start;
    size_t size;
} Heap_Block;

typedef struct
{
    size_t count;
    Heap_Block blocks[HEAP_BLOCK_LIST_CAPACITY];
} Heap_Block_List;

/* ── Global state (defined in main.c) ── */
extern char heap_buffer[HEAP_CAPACITY];
extern Heap_Block_List heap_allocated_blocks;
extern Heap_Block_List heap_freed_blocks;

/* ── Allocator API ── */
void *heap_alloc(size_t size);
void  heap_free(void *ptr);
void  heap_coalesce_free_blocks(void);
void  gc_collect(void *root);

/* ── Block list operations ── */
void chunk_list_add(Heap_Block_List *list, void *start, size_t size);
int  chunk_list_find(const Heap_Block_List *list, void *ptr);
void chunk_list_remove(Heap_Block_List *list, size_t index);
int  chunk_compare(const void *a, const void *b);

#endif /* HEAP_H */
