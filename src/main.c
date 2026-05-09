#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#define HEAP_CAPACITY 1280000
#define HEAP_BLOCK_LIST_CAPACITY 4096

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
    Heap_Block blocks[HEAP_BLOCK_LIST_CAPACITY];
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

int chunk_compare(const void *a, const void *b)
{
    const Heap_Block *block_a = (const Heap_Block *)a;
    const Heap_Block *block_b = (const Heap_Block *)b;
    return block_a->start - block_b->start;
}

int chunk_list_find(const Heap_Block_List *list, void *ptr)
{
    Heap_Block key = {.start = ptr};
    Heap_Block *res = bsearch(&key, list->blocks,
                              list->count, list->blocks[0].size,
                              chunk_compare);
    if (res != 0)
    {
        assert(list->blocks <= res);
        return (res - list->blocks) / sizeof(list->blocks[0]);
    }
}

void chunk_list_remove(Heap_Block_List *list, size_t index)
{
    assert(index < list->count && "chunk_list_remove: index out of bounds");

    for (size_t i = index; i < list->count - 1; i++)
    {
        list->blocks[i] = list->blocks[i + 1];
    }
    list->count--;
}

void chunk_list_add(Heap_Block_List *list, void *start, size_t size)
{
    assert(list->count < HEAP_BLOCK_LIST_CAPACITY && "chunk_list_add: block list capacity exceeded");

    list->blocks[list->count].start = start;
    list->blocks[list->count].size = size;

    // we perform sorting on every add to keep the list ordered by start address
    for (size_t i = list->count - 1; i > 0; i--)
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
    if (ptr == NULL)
        return;

    const int index = chunk_list_find(&heap_allocated_blocks, ptr);

    assert(index >= 0 && "heap_free: pointer not found in allocated blocks");

    chunk_list_add(&heap_freed_blocks, heap_allocated_blocks.blocks[index].start, heap_allocated_blocks.blocks[index].size);

    chunk_list_remove(&heap_allocated_blocks, index);
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
        if (i % 2 == 0)
        {
            heap_free(p);
        }
    }

    chunk_list_dump(&heap_allocated_blocks);
    chunk_list_dump(&heap_freed_blocks);

    return 0;
}