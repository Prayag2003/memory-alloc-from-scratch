#include <stdio.h>
#include <assert.h>

#define HEAP_CAPACITY 1280000
#define HEAP_ALLOCATED_BLOCKS_CAPACITY 4096

/* Internal Heap Storage */
static char heap_buffer[HEAP_CAPACITY] = {0};
size_t heap_size = 0;

typedef struct
{
    void *start;
    size_t size;
} Heap_Block;

Heap_Block heap_allocated_blocks[HEAP_ALLOCATED_BLOCKS_CAPACITY];
size_t heap_allocated_blocks_size = 0;

static int
heap_has_space(size_t n)
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

    const Heap_Block block = {
        .start = ptr,
        .size = size,
    };
    assert(heap_allocated_blocks_size < HEAP_ALLOCATED_BLOCKS_CAPACITY && "heap_alloc: too many allocated blocks");

    heap_allocated_blocks[heap_allocated_blocks_size++] = block;

    return ptr;
}

void heap_dump_allocated_blocks(void)
{
    printf("Allocated blocks: %zu\n", heap_allocated_blocks_size);
    for (size_t i = 0; i < heap_allocated_blocks_size; i++)
    {
        printf("Block %zu: start=%p, size=%zu\n", i, heap_allocated_blocks[i].start, heap_allocated_blocks[i].size);
    }
}

void heap_free(void *ptr)
{
    (void)ptr;
    assert(0 && "heap_free is not implemented");
}

void gc_collect(void *root)
{
    (void)root;
    assert(0 && "gc_collect is not implemented");
}

int main(void)
{
    for (int i = 0; i < 10; i++)
        heap_alloc(i);

    heap_dump_allocated_blocks();
    // heap_free(root);

    return 0;
}