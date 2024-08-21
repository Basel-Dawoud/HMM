# Heap Memory Manager (HMM) and Linked List Implementation

## Overview

This project implements a Heap Memory Manager (HMM) in C to simulate dynamic memory allocation in user space. It also includes a basic linked list implementation that uses the HMM for memory management. 

### Features

- **Heap Memory Manager (HMM)**
  - Simulates heap allocation and deallocation.
  - Supports `HmmAlloc()` for allocating memory and `HmmFree()` for freeing memory.
  - Implements a basic free list for managing available memory blocks.

- **Linked List Operations**
  - Supports insertion at the beginning, end, and after a specific node.
  - Supports deletion from the beginning, end, and by node value.
  - Uses the HMM to allocate and deallocate memory for nodes.

## File Structure

- `HMM_Main.c`: The main file containing the implementation of the Heap Memory Manager and linked list operations.
- `README.md`: This file, providing documentation and instructions.

## Implementation Details

### Heap Memory Manager

The heap memory manager simulates a memory heap using a statically allocated array. The following functions are implemented:

1. **`void *sbreak(void)`**
   - Simulates the system call to get the current program break.

2. **`void *HmmAlloc(size_t size)`**
   - Allocates memory of the given size from the simulated heap. Uses a first-fit strategy to find a suitable block and splits blocks if necessary.

3. **`void HmmFree(void *ptr)`**
   - Frees the memory at the given pointer and updates the free list. Merges adjacent free blocks if possible.

4. **`void updateFreeList(struct fnode *node)`**
   - Inserts a freed block into the free list and merges adjacent free blocks.

5. **`void freeListInit(void)`**
   - Initializes the free list with a single large block representing the entire heap.

6. **`void printFreeList(void)`**
   - Prints the current state of the free list for debugging purposes.

### Linked List Operations

The linked list operations use the heap manager for memory allocation. The following functions are implemented:

1. **`void insertatbegin(int data)`**
   - Inserts a new node at the beginning of the linked list.

2. **`void insertatend(int data)`**
   - Inserts a new node at the end of the linked list.

3. **`void insertafternode(struct node *list, int data)`**
   - Inserts a new node after the specified node.

4. **`void deleteatbegin(void)`**
   - Deletes the first node of the linked list.

5. **`void deleteatend(void)`**
   - Deletes the last node of the linked list.

6. **`void deletenode(int key)`**
   - Deletes the node with the specified value.

7. **`void printList(void)`**
   - Prints the linked list.

## How to Compile and Run

### Compilation

To compile the `HMM_Main.c` file, use a C compiler such as `gcc`:

```sh
gcc -o hmm_main HMM_Main.c
```

### Running

After compilation, run the executable:

```sh
./hmm_main
```

## Example Output

When running the program, you can expect output similar to the following:

```
Testing Heap Memory Manager...
Allocated 100 bytes at 0x... 
Allocated 200 bytes at 0x... 
Allocated 300 bytes at 0x... 
Allocated 50 bytes at 0x... 
Allocated 150 bytes at 0x... 
Free List:
Block of size 0 at 0x...
Block of size 0 at 0x...

Testing Linked List...
Linked List after insertions: [ 30  25  40  50 ]
Linked List after deletions: [ 25  40 ]
```

## Design Decisions

1. **Memory Alignment**: Allocated sizes are aligned to the nearest multiple of `sizeof(size_t)` to ensure proper memory alignment.
2. **Block Splitting**: When allocating memory, blocks are split if the remaining free block size is large enough.
3. **Free List Merging**: Freed blocks are merged with adjacent free blocks to minimize fragmentation.

## Known Issues and Limitations

- The current implementation does not handle all edge cases and is intended for educational purposes.
- The free list merging strategy is basic and might not handle all fragmentation scenarios optimally.
