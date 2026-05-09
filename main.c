#include <stdio.h>
#include <assert.h>

#define HEAP_CAPACITY 1280000

/* Internal Heap Storage */
static char heap_buffer[HEAP_CAPACITY] = {0};
size_t heap_size = 0;

static int heap_has_space(size_t n)
{
    return heap_size + n <= HEAP_CAPACITY;
}

/* Allocate 'size' bytes from our linear heap */
void *heap_alloc(size_t size)
{
    assert(heap_has_space(size) && "heap_alloc: out of memory");

    void *result = heap_buffer + heap_size;
    heap_size += size;
    return result;
}

int main()
{
    // a pointer in stack that points to the heap
    char *root = heap_alloc(10);
    for (int i = 0; i < 10; i++)
    {
        root[i] = '1' + i;
    }
    return 0;
}