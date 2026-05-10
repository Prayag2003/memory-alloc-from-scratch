# 🧠 Memory Allocator — From Scratch

A minimal, educational **first-fit free-list memory allocator** written in C17. No `malloc`, no `sbrk` — just a static buffer and manual bookkeeping.

---

## Table of Contents

- [Output](#output)
- [Theory](#theory)
  - [What Is a Heap?](#what-is-a-heap)
  - [Free List](#free-list)
  - [First-Fit Allocation](#first-fit-allocation)
  - [Block Splitting](#block-splitting)
  - [Coalescing](#coalescing)
  - [Fragmentation](#fragmentation)
  - [Reuse Detection](#reuse-detection)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Build & Run](#build--run)
- [Roadmap](#roadmap)

---

## Output

### Basic Allocation & Freeing

The allocator runs a test that allocates 10 blocks (sizes 0–9), frees the even-indexed ones, and displays the resulting heap state with a visual memory map.

![Basic allocation and free demo](./assets/demo.png)

### Memory Reuse

When previously freed blocks are reallocated, the allocator detects this and marks the log with **(REUSED)** — confirming that holes in the heap are being reclaimed rather than wasting memory at the tail.

![Reuse detection in action](./assets/reused.png)

### Larger Allocation

A 100-byte allocation is placed into the heap's free tail region. The heap visualizer clearly shows the allocated portion growing relative to the total capacity.

![Larger allocation example](./assets/bigger_allocation.png)

---

## Theory

### What Is a Heap?

In most programs, `malloc` and `free` manage a region of memory called the **heap**. The OS gives the process a contiguous chunk of address space, and the allocator is responsible for carving it up into smaller blocks on demand.

This project simulates a heap using a **static array**:

```c
char heap_buffer[128000];  // 128 KB of raw memory
```

The allocator manages this buffer by maintaining two sorted lists — one for **allocated blocks** and one for **free blocks**. Every byte in the buffer belongs to exactly one of these lists at any given time.

```
┌────────────────────────────────────────────────────────────┐
│                    heap_buffer (128 KB)                     │
│  ┌──────┬──────┬──────┬──────┬──────┬────────────────────┐ │
│  │ used │ free │ used │ free │ used │    free (tail)      │ │
│  └──────┴──────┴──────┴──────┴──────┴────────────────────┘ │
└────────────────────────────────────────────────────────────┘
```

### Free List

The allocator tracks every free region as a `Heap_Block`:

```c
typedef struct {
    void  *start;   // pointer to the first byte of this block
    size_t size;    // number of bytes in this block
} Heap_Block;
```

All free blocks are kept in a **sorted array** (sorted by `start` address). Sorting enables:
- **Binary search** (`bsearch`) for O(log n) lookups when freeing
- **Linear scan** for first-fit allocation
- **Coalescing** — adjacent free blocks are always neighbors in the array

Initially, the entire heap is one giant free block:

```c
Heap_Block_List heap_freed_blocks = {
    .count = 1,
    .blocks = { [0] = { .start = heap_buffer, .size = 128000 } }
};
```

### First-Fit Allocation

When `heap_alloc(size)` is called, the allocator scans the free list **from the beginning** and picks the **first block that is large enough**:

```
Free list:  [2 bytes] → [8 bytes] → [4 bytes] → [127900 bytes]
                            ↑
Request: alloc(5)      fits here! (8 ≥ 5)
```

**Why first-fit?**

| Strategy  | Behavior                         | Trade-off                       |
|-----------|----------------------------------|---------------------------------|
| First-fit | Take the first block that fits   | Fast, but can fragment the front |
| Best-fit  | Take the smallest fitting block  | Less waste, but slower scan      |
| Worst-fit | Take the largest block           | Leaves big chunks, but fragments |

First-fit is the simplest and often performs surprisingly well in practice.

### Block Splitting

If the chosen free block is **larger** than requested, the allocator **splits** it:

```
Before:   [████████ 8 bytes free ████████]

alloc(3)

After:    [███ 3B alloc ███][█████ 5B free █████]
```

The first portion becomes allocated; the leftover tail stays in the free list. This ensures no memory is wasted beyond what's requested.

### Coalescing

When a block is freed, it's inserted back into the free list. The allocator then checks for **adjacent free blocks** and merges them:

```
Before free(B):
  [AAAA][BBBB][....free....]

After free(B):
  [AAAA][........merged free........]
       ↑ B merges with the free block to its right
```

Without coalescing, the free list would accumulate tiny fragments that can never satisfy larger allocations — even though the total free memory is sufficient. This is the classic **external fragmentation** problem.

The coalescing pass runs in O(n) after every `free`:

```c
void heap_coalesce_free_blocks(void) {
    for (size_t i = 0; i + 1 < heap_freed_blocks.count; ) {
        Heap_Block *curr = &heap_freed_blocks.blocks[i];
        Heap_Block *next = &heap_freed_blocks.blocks[i + 1];
        if ((char *)curr->start + curr->size == (char *)next->start) {
            curr->size += next->size;   // merge
            chunk_list_remove(&heap_freed_blocks, i + 1);
        } else {
            i++;
        }
    }
}
```

### Fragmentation

Even with coalescing, **external fragmentation** can still occur when allocated blocks separate free regions that can't be merged:

```
[alloc 1B][free 1B][alloc 3B][free 2B][alloc 5B][free 3B][alloc 7B]...
```

The total free space might be 6 bytes, but the largest contiguous block is only 3 — so `alloc(4)` would fail. This is an inherent limitation of any allocator that doesn't move blocks (no compaction).

**Internal fragmentation** is minimal in this allocator because we split blocks exactly. The only waste is the metadata overhead (two pointers per tracked block).

### Reuse Detection

The allocator tracks a **high-water mark** — the furthest point in the buffer that has ever been allocated. When a new allocation lands **below** this mark, it means the allocator is filling a hole left by a previous `free()`, and the log is tagged with **(REUSED)**:

```
[ALLOC] 1 bytes at 0x102450011 (REUSED)    ← filling a freed hole
[ALLOC] 2 bytes at 0x102450016 (REUSED)    ← filling a freed hole
[ALLOC] 5 bytes at 0x10245003d             ← extending into fresh heap
```

This provides a clear visual indicator of how efficiently the allocator reclaims freed memory.

---

## How It Works

- **`heap_alloc(size)`** — First-fit scan of the free list; splits the block if it's larger than requested
- **`heap_free(ptr)`** — Finds the block via `bsearch`, moves it to the free list, then coalesces adjacent free blocks
- **`heap_coalesce_free_blocks()`** — Merges touching free blocks to combat fragmentation
- **`heap_visualize()`** — Renders a colored ASCII bar (`█` = allocated, `░` = free) with heap statistics
- Block lists stay sorted by address (insertion sort on add, `bsearch` on find)

---

## Project Structure

```
├── src/
│   ├── main.c        # Allocator logic (alloc, free, coalesce, block ops) + test driver
│   ├── heap.h        # Types, constants, extern declarations
│   ├── display.h     # ANSI color macros, logging macros, display API
│   └── display.c     # Table dump, heap visualizer, banner
├── assets/
│   ├── demo.png              # Basic allocation & free output
│   ├── reused.png            # Memory reuse detection output
│   └── bigger_allocation.png # Larger allocation example
├── Makefile
├── TODOS.txt
└── README.md
```

---

## Build & Run

```bash
make run      # build + run
make          # build only
make debug    # launch with lldb
make clean    # remove binary
```

Requires a C17 compiler (`clang` by default).

---

## Roadmap

- [x] First-fit free-list allocator
- [x] Block splitting on allocation
- [x] Block coalescing on free
- [x] Sorted block lists (insertion sort + `bsearch`)
- [x] Heap visualizer (ASCII bar + stats)
- [x] Reuse detection logging
- [ ] Best-fit / worst-fit strategies
- [ ] Thread safety (mutex locking)
- [ ] Garbage collector (`gc_collect` is stubbed)
- [ ] Benchmarks vs system `malloc`