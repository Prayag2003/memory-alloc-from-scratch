#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "heap.h"
#include "display.h"

/* Internal Heap Storage */
char heap_buffer[HEAP_CAPACITY] = {0};

Heap_Block_List heap_allocated_blocks = {0};
Heap_Block_List heap_freed_blocks = {
    .count = 1, 
    .blocks = {
        [0] = {.start = heap_buffer, .size = sizeof(heap_buffer)}}
};

/*
 * Comparator for two Heap_Blocks by start address.
 * Used by bsearch / qsort to keep blocks ordered.
 */
int chunk_compare(const void *a, const void *b)
{
    const Heap_Block *block_a = (const Heap_Block *)a;
    const Heap_Block *block_b = (const Heap_Block *)b;
    return block_a->start - block_b->start;
}

/*
 * Binary-searches the block list for a block whose
 * start address matches 'ptr'.
 * Returns the index if found; undefined if not found.
 */
int chunk_list_find(const Heap_Block_List *list, void *ptr)
{
    Heap_Block key = {.start = ptr};
    Heap_Block *res = bsearch(&key, list->blocks,
                              list->count, sizeof(list->blocks[0]),
                              chunk_compare);
    if (res != NULL)
    {
        assert(list->blocks <= res);
        return (int)(res - list->blocks);
    }
    return -1;
}

/*
 * Removes the block at 'index' by shifting all
 * subsequent blocks one position to the left.
 */
void chunk_list_remove(Heap_Block_List *list, size_t index)
{
    assert(index < list->count && "chunk_list_remove: index out of bounds");

    for (size_t i = index; i < list->count - 1; i++)
    {
        list->blocks[i] = list->blocks[i + 1];
    }
    list->count--;
}

/*
 * Appends a new block and insertion-sorts it so the
 * list stays ordered by start address.
 */
void chunk_list_add(Heap_Block_List *list, void *start, size_t size)
{
    assert(list->count < HEAP_BLOCK_LIST_CAPACITY && "chunk_list_add: block list capacity exceeded");

    list->blocks[list->count].start = start;
    list->blocks[list->count].size = size;

    // we perform sorting on every add to keep the list ordered by start address
    for (size_t i = list->count; i > 0; i--)
    {
        if (list->blocks[i - 1].start > list->blocks[i].start)
        {
            Heap_Block temp = list->blocks[i];
            list->blocks[i] = list->blocks[i - 1];
            list->blocks[i - 1] = temp;
        }
        else
            break;
    }
    list->count++;
}

/*
 * First-fit allocation: scans the free list for the
 * first block large enough, splits it if needed, and
 * moves the allocated portion to the allocated list.
 * Returns NULL when no suitable free block exists.
 */
void *heap_alloc(size_t size)
{
    if (size <= 0)
    {
        LOG_ERROR("alloc of 0 bytes ignored");
        return NULL;
    }

    for(size_t i = 0; i < heap_freed_blocks.count; i++){
        const Heap_Block block = heap_freed_blocks.blocks[i];
        if(block.size >= size){
            chunk_list_remove(&heap_freed_blocks, i);
            void *ptr = block.start;
            size_t tail_size = block.size - size;
        
            chunk_list_add(&heap_allocated_blocks, ptr, size);

            if(tail_size > 0){
                chunk_list_add(&heap_freed_blocks, ptr + size, tail_size);
            }
            LOG_ALLOC(ptr, size);
            return ptr;
        }
    }
    LOG_ERROR("out of memory — no free block large enough");
    return NULL;
}

/*
 * Frees a previously allocated block.
 * Finds it in the allocated list, moves it
 * to the freed list, and removes it from allocated.
 */
void heap_free(void *ptr)
{
    if (ptr == NULL)
    {
        LOG_ERROR("free(NULL) ignored");
        return;
    }

    const int index = chunk_list_find(&heap_allocated_blocks, ptr);

    assert(index >= 0 && "heap_free: pointer not found in allocated blocks");

    size_t freed_size = heap_allocated_blocks.blocks[index].size;
    chunk_list_add(&heap_freed_blocks, heap_allocated_blocks.blocks[index].start, freed_size);

    chunk_list_remove(&heap_allocated_blocks, index);
    LOG_FREE(ptr, freed_size);

    /* Coalesce adjacent free blocks to reduce fragmentation */
    heap_coalesce_free_blocks();
}

/*
 * Merges adjacent free blocks into larger ones.
 * The free list is kept sorted by start address,
 * so we just scan for consecutive blocks that touch.
 */
void heap_coalesce_free_blocks(void)
{
    for (size_t i = 0; i + 1 < heap_freed_blocks.count; )
    {
        Heap_Block *curr = &heap_freed_blocks.blocks[i];
        Heap_Block *next = &heap_freed_blocks.blocks[i + 1];

        if ((char *)curr->start + curr->size == (char *)next->start)
        {
            /* Merge 'next' into 'curr' */
            curr->size += next->size;
            chunk_list_remove(&heap_freed_blocks, i + 1);
        }
        else
        {
            i++;
        }
    }
}

/*
 * Placeholder for a future garbage collector.
 * Not yet implemented — will abort if called.
 */
void gc_collect(void *root)
{
    (void)root;
    assert(0 && "gc_collect is not implemented");
}

int main(void)
{
    print_banner();

    LOG_INFO("Starting allocation test (10 blocks, freeing evens)...");
    printf("\n");

    for (int i = 0; i < 10; i++)
    {
        void *p = heap_alloc(i);
        if (i % 2 == 0)
        {
            heap_free(p);
        }
    }
    
    // heap_alloc(100);

    for(int i = 1; i <= 4; i++){
        heap_alloc(i); 
    }

    chunk_list_dump(CLR_GREEN "Allocated Blocks", CLR_GREEN, &heap_allocated_blocks);
    chunk_list_dump(CLR_YELLOW "Free Blocks", CLR_YELLOW, &heap_freed_blocks);

    heap_visualize();

    printf("\n");
    return 0;
}