# Heap Memory Allocator System (HMM)

## Overview

This repository contains a simulation of a heap memory management system. It includes functionality for allocating and freeing memory blocks from a simulated heap using a custom memory management algorithm. The core functionality revolves around maintaining a linked list of free memory blocks and managing these blocks through allocation and deallocation processes.


## Description

The code implements a custom memory allocator using a simulated heap represented by a static array. It supports dynamic memory allocation and deallocation with a first-fit strategy. The implementation maintains a free list of memory blocks and supports merging of adjacent free blocks to optimize memory usage.

## Heap Structure

The simulated heap is represented by a static array `vHeap` of fixed size (`VHEAP_MAX_SIZE`). The `heapBase` and `programBreak` pointers are used to manage and simulate the heap boundaries. 

### Key Data Structures

- **`fnode`**: Represents a free block in the heap.
    - `length`: Size of the block.
    - `prev`: Pointer to the previous block in the free list.
    - `next`: Pointer to the next block in the free list.

## Functions

### `sbreak`

```c
void *sbreak(intptr_t increment);
```

**Purpose**: Adjusts the simulated program break (end of heap).

**Description**: Moves the program break by the specified `increment`. It ensures that the new program break does not exceed the limits of the simulated heap. If the new break exceeds the heap boundaries, it returns an error.

### `freeListInit`

```c
void freeListInit(void);
```

**Purpose**: Initializes the free list with a single large block.

**Description**: Sets the head of the free list to the start of the simulated heap and initializes the first node with the full size of the heap. Updates the tail to point to the same node and sets the `prev` and `next` pointers accordingly.

### `insertend`

```c
void insertend(size_t neweSize);
```

**Purpose**: Adds a new free block at the end of the free list.

**Description**: Ensures that the new block does not exceed the heap boundary. Calculates the position for the new block, updates the pointers of the previous tail node, and sets the length of the new block. 

### `mergeNodes`

```c
void mergeNodes(void);
```

**Purpose**: Merges adjacent free blocks into a single larger block.

**Description**: Iterates through the free list, merging nodes that are adjacent to each other. Updates the `length` and `next` pointers accordingly. If the last block is merged, updates the tail pointer.

### `addafternode`

```c
void* addafternode(fnode* node);
```

**Purpose**: Adds a new free node immediately after the specified node.

**Description**: Calculates the position for the new node based on the given node's length. Updates the `prev` and `next` pointers of the new node and its adjacent nodes.

### `split`

```c
void split(fnode* node, size_t blockSize);
```

**Purpose**: Splits a free block into two blocks if it is larger than the requested size.

**Description**: Adjusts the size of the given block to the requested size and creates a new block for the remaining space. Updates the free list to include the new block and maintains the proper pointers.

### `firstFit`

```c
void *firstFit(size_t blockSize);
```

**Purpose**: Finds a free block that fits the requested size using the first-fit strategy.

**Description**: Searches the free list for the first block that can accommodate the requested size. If found, it splits the block if necessary and returns a pointer to the block.

### `HmmAlloc`

```c
void *HmmAlloc(size_t blockSize);
```

**Purpose**: Allocates a memory block of the requested size.

**Description**: Aligns the block size to a multiple of 8 bytes, calculates the total size needed (including metadata), and finds a suitable block using the `firstFit` function. If no suitable block is found, it expands the heap using `sbreak` and inserts a new block at the end. Removes the allocated block from the free list and returns a pointer to the usable memory (skipping the metadata).

### `HmmFree`

```c
void HmmFree(void *ptr);
```

**Purpose**: Frees a previously allocated memory block.

**Description**: Converts the pointer to a `fnode` structure and inserts it into the free list as the new head. Updates the pointers of the old head and merges adjacent free blocks using `mergeNodes`.

## Usage

To use this memory management system, include the `heap.h` header file in your project. Call `HmmAlloc` to allocate memory and `HmmFree` to deallocate memory. The system automatically manages the free list and merges adjacent free blocks.

**Example**:

```c
#include "heap.h"

int main() {
    // Allocate a block of memory
    void *ptr = HmmAlloc(100);
    
    // Check if allocation was successful
    if (ptr) {
        // Use the allocated memory
    }
    
    // Free the allocated memory
    HmmFree(ptr);
    
    return 0;
}
```

## Testing

The repository includes a stress_test and test2 for random allocation and deallocation scenarios. To run the tests, compile the source code and execute the test program.
