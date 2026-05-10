#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "heap.h"
#include "display.h"

/* Internal Heap Storage */
char heap_buffer[HEAP_CAPACITY] = {0};

MemBlockList allocated_blocks = {0};
MemBlockList free_blocks = {
    .count = 1, 
    .blocks = {
        [0] = {.start = heap_buffer, .size = sizeof(heap_buffer)}}
};

/* High-water mark: tracks the furthest point the heap has ever allocated to.
 * Any allocation below this address is reusing previously freed memory. */
static void *heap_high_water = heap_buffer;

/*
 * Comparator for two MemBlocks by start address.
 * Used by bsearch / qsort to keep blocks ordered.
 */
int block_compare(const void *a, const void *b)
{
    const MemBlock *block_a = (const MemBlock *)a;
    const MemBlock *block_b = (const MemBlock *)b;
    return (char *)block_a->start - (char *)block_b->start;
}

/*
 * Binary-searches the block list for a block whose
 * start address matches 'ptr'.
 * Returns the index if found; undefined if not found.
 */
int block_list_find(const MemBlockList *list, void *ptr)
{
    MemBlock key = {.start = ptr};
    MemBlock *res = bsearch(&key, list->blocks,
                              list->count, sizeof(list->blocks[0]),
                              block_compare);
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
void block_list_remove(MemBlockList *list, size_t index)
{
    assert(index < list->count && "block_list_remove: index out of bounds");

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
void block_list_add(MemBlockList *list, void *start, size_t size)
{
    assert(list->count < HEAP_BLOCK_LIST_CAPACITY && "block_list_add: block list capacity exceeded");

    list->blocks[list->count].start = start;
    list->blocks[list->count].size = size;

    // we perform sorting on every add to keep the list ordered by start address
    for (size_t i = list->count; i > 0; i--)
    {
        if (list->blocks[i - 1].start > list->blocks[i].start)
        {
            MemBlock temp = list->blocks[i];
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

    for(size_t i = 0; i < free_blocks.count; i++){
        const MemBlock block = free_blocks.blocks[i];
        if(block.size >= size){
            block_list_remove(&free_blocks, i);
            void *ptr = block.start;
            size_t tail_size = block.size - size;
        
            block_list_add(&allocated_blocks, ptr, size);

            if(tail_size > 0){
                block_list_add(&free_blocks, (char *)ptr + size, tail_size);
            }

            /* Detect reuse: if this address is below the high-water mark,
             * we're filling a hole left by a previous free(). */
            if ((char *)ptr + size > (char *)heap_high_water)
            {
                heap_high_water = (char *)ptr + size;
                LOG_ALLOC(ptr, size);
            }
            else
            {
                LOG_ALLOC_REUSE(ptr, size);
            }
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

    const int index = block_list_find(&allocated_blocks, ptr);

    assert(index >= 0 && "heap_free: pointer not found in allocated blocks");

    size_t freed_size = allocated_blocks.blocks[index].size;
    block_list_add(&free_blocks, allocated_blocks.blocks[index].start, freed_size);

    block_list_remove(&allocated_blocks, index);
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
    for (size_t i = 0; i + 1 < free_blocks.count; )
    {
        MemBlock *curr = &free_blocks.blocks[i];
        MemBlock *next = &free_blocks.blocks[i + 1];

        if ((char *)curr->start + curr->size == (char *)next->start)
        {
            /* Merge 'next' into 'curr' */
            curr->size += next->size;
            block_list_remove(&free_blocks, i + 1);
        }
        else
        {
            i++;
        }
    }
}

int main(void)
{
    print_banner();

    LOG_INFO("Starting simple allocation & reuse test...");
    printf("\n");

    /* 1. Allocate 3 blocks sequentially */
    void *a = heap_alloc(10);
    void *b = heap_alloc(20);
    void *c = heap_alloc(30);

    printf("\n");
    LOG_INFO("Freeing the middle block (20 bytes) to create a hole...");
    printf("\n");

    /* 2. Free the middle one to create a hole */
    heap_free(b);

    printf("\n");
    LOG_INFO("Allocating a new 15-byte block (should reuse the hole!)...");
    printf("\n");

    /* 3. Allocate something smaller than the hole — it should reuse it */
    void *d = heap_alloc(15);


    block_list_dump(CLR_GREEN "Allocated Blocks", CLR_GREEN, &allocated_blocks);
    block_list_dump(CLR_YELLOW "Free Blocks", CLR_YELLOW, &free_blocks);

    heap_visualize();

    printf("\n  Press [ENTER] to start live memory simulation...\n");
    getchar();

    g_logging_enabled = 0;
    void *sim_ptrs[40] = {0};

    for (int step = 1; step <= 20; step++) {
        if (step > 1) {
            printf("\033[15A\033[J");
        }
        
        printf("  %s[Live Simulation Step %d / 20]%s\033[K\n", CLR_MAGENTA, step, CLR_RESET);

        int r = rand() % 10;
        if (r < 7) {
            int idx = -1;
            for(int i = 0; i < 40; i++) {
                if(!sim_ptrs[i]) { idx = i; break; }
            }
            if (idx != -1) {
                size_t sz = (rand() % 4000) + 10;
                sim_ptrs[idx] = heap_alloc(sz);
            }
        } else {
            for(int i = 0; i < 40; i++) {
                int rnd_idx = rand() % 40;
                if(sim_ptrs[rnd_idx]) { 
                    heap_free(sim_ptrs[rnd_idx]);
                    sim_ptrs[rnd_idx] = NULL;
                    break;
                }
            }
        }

        heap_visualize();
        usleep(400000); // 400 ms delay
    }

    g_logging_enabled = 1;
    printf("\n  %sSimulation Complete!%s\n\n", CLR_GREEN, CLR_RESET);

    return 0;
}