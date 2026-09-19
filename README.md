# Custom Memory Allocator (malloc/free/realloc from scratch on UNIX)

A from-scratch reimplementation of `malloc`, `free`, and `realloc` in C, built to actually understand what the standard library is doing under the hood instead of just trusting it. Memory is requested from the OS with `sbrk`, and everything past that — tracking blocks, finding a free one to reuse, growing an allocation — is handled manually with an intrusive linked list.

It also has a little `ncurses` terminal UI on top so you can watch the heap change live as you run operations, instead of squinting at printed addresses.

## Why

`malloc` is one of those things everyone uses and almost nobody has actually built. This project was mostly an excuse to get hands-on with pointer arithmetic, struct layout, and the kind of bookkeeping tricks real allocators use.

## How it works

Each allocated block looks like this in memory:

```
[ header (size, free flag, next pointer) ][ usable memory returned to the caller ]
```

The header sits directly in front of the memory it describes — this is an "intrusive" linked list, so there's no separate bookkeeping structure off to the side. `malloc(size)` walks this list looking for a free block big enough to reuse (first-fit). If nothing fits, it calls `sbrk` to grow the heap and appends a new block to the list.

`free(ptr)` doesn't reclaim or erase anything,it just walks back from the payload pointer to its header and flips a flag. `realloc(ptr, new_size)` either reuses the block in place if it's already big enough, or allocates a new block, copies the old data over with `memcpy`, and frees the original.

This is intentionally the simple version — no block splitting, no coalescing of adjacent free blocks, no alignment handling. Real allocators solve for these; this project is about understanding the core mechanism first.

## The terminal UI

Run the program and you get a small menu:

```
| OPTIONS |
1. malloc()
2. free()
3. realloc()
4. exit
```

Below it, the current state of the heap is drawn live — one row per block, color-coded green for allocated and red for free — so every action is immediately visible. Free a block in the middle of a chain of allocations and you'll see it turn red right there without disappearing from the list, which is a pretty good way to *see* fragmentation happening instead of just being told about it.

## Building it

Needs `ncurses` installed.

## linux kernal devices(at least the ones I tested it :D,only ubuntu ):

you can download ncurses with a basic ```sudo apt install -y libncurses-dev ``` command. 

Then comp:

```bash
gcc allocator.c -o allocator -lncurses
./allocator
