# 🧠 Memory Allocator — From Scratch

A minimal, educational **first-fit free-list memory allocator** written in C17. No `malloc`, no `sbrk` — just a static buffer and manual bookkeeping.

---

## Table of Contents

- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Build & Run](#build--run)
- [Roadmap](#roadmap)

---

## How It Works

```
┌─────────────────────────────────────────────────┐
│              heap_buffer (128 KB)                │
│  ┌──────┬──────┬──────┬──────┬──────────────┐   │
│  │ used │ free │ used │ free │  free (tail)  │   │
│  └──────┴──────┴──────┴──────┴──────────────┘   │
└─────────────────────────────────────────────────┘
```

- **`heap_alloc(size)`** — First-fit scan of the free list, splits the block if larger than requested
- **`heap_free(ptr)`** — Looks up the block via binary search, moves it back to the free list
- **`heap_visualize()`** — Renders a colored ASCII bar of allocated vs free regions
- Block lists stay sorted by address (insertion sort on add, `bsearch` on find)

---

## Project Structure

```
├── src/
│   ├── main.c        # Allocator logic (alloc, free, block ops) + test driver
│   ├── heap.h        # Types, constants, extern declarations
│   ├── display.h     # ANSI color macros, logging macros, display API
│   └── display.c     # Table dump, heap visualizer, banner
├── assets/
│   └── output.png    # Terminal output screenshot
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

- [ ] Block coalescing (merge adjacent free blocks)
- [ ] Best-fit / worst-fit strategies
- [ ] Thread safety (mutex locking)
- [ ] Garbage collector (`gc_collect` is stubbed)
- [ ] Benchmarks vs system `malloc`