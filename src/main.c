#include <stdio.h>
#include <assert.h>

#define HEAP_CAPACITY 1280000
#define HEAP_ALLOCATED_BLOCKS_CAPACITY 4096
#define HEAP_FREED_BLOCKS_CAPACITY 4096

/* Internal Heap Storage */
static char heap_buffer[HEAP_CAPACITY] = {0};
size_t heap_size = 0;

typedef struct
{
    void *start;
    size_t size;
} Heap_Block;

typedef struct
{
    size_t count;
    Heap_Block blocks[HEAP_ALLOCATED_BLOCKS_CAPACITY];
} Heap_Block_List;

Heap_Block_List heap_allocated_blocks = {0};
Heap_Block_List heap_freed_blocks = {0};

void chunk_list_dump(const Heap_Block_List *list)
{
    printf("Allocated blocks: %zu\n", list->count);
    for (size_t i = 0; i < list->count; i++)
    {
        printf("Block %zu: start=%p, size=%zu\n", i, list->blocks[i].start, list->blocks[i].size);
    }
}

int chunk_list_find(const Heap_Block_List *list, void *ptr)
{
}

void chunk_list_remove(Heap_Block_List *list, size_t index)
{
}

void chunk_list_add(Heap_Block_List *list, void *start, size_t size)
{
}

static int heap_has_space(size_t n)
{
    return heap_size + n <= HEAP_CAPACITY;
}

/* Allocate 'size' bytes from our linear heap */
void *heap_alloc(size_t size)
{
    if (size <= 0)
        return NULL;

    assert(heap_has_space(size) && "heap_alloc: out of memory");

    void *ptr = heap_buffer + heap_size;
    heap_size += size;
    chunk_list_add(&heap_allocated_blocks, ptr, size);
    return ptr;
}

void heap_free(void *ptr)
{
}

void gc_collect(void *root)
{
    (void)root;
    assert(0 && "gc_collect is not implemented");
}

int main(void)
{
    for (int i = 0; i < 10; i++)
    {
        void *p = heap_alloc(i);
        if (i & 1)
        {
            heap_free(p);
        }
    }

    // chunk_list_dump();
    // heap_free(root);

    return 0;
}