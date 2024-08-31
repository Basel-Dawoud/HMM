
# Heap Memory Allocator System (HMM)

## Overview

This repository contains a simulation of a heap memory management system. It includes functionality for allocating and freeing memory blocks from a simulated heap using a custom memory management algorithm. The core functionality revolves around maintaining a linked list of free memory blocks and managing these blocks through allocation and deallocation processes.


## Description

The code implements a custom memory allocator using a simulated heap represented by a static array. It supports dynamic memory allocation and deallocation with a first-fit strategy. The implementation maintains a free list of memory blocks and supports merging of adjacent free blocks to optimize memory usage.

### **Flow Chart Diagram**

```
                              +-------------------+
                              |       Start       |
                              +-------------------+
                                        |
                                        v
                              +-------------------+
                              | Initialize HMM    |
                              | - Static Array    |
                              | - Simulated Break |
                              +-------------------+
                                        |
                                        v
                              +--------------------------+
                              | User Request for Memory  |
                              +--------------------------+
                                        |
                                        v
                                 +---------------------------+
                                 |   Is there enough space?  |
                                 +---------------------------+
                                   | No                     | Yes
                                   v                        v
                              +-------------------+    +---------------------------+
                              | Error Handling    |    | Allocate Memory           |
                              | - Output Error    |    +---------------------------+
                              +-------------------+                |
                                        |                          |
                                        v                          v
                                      End            +----------------------------+
                                                     | Update Simulated Break     |
                                                     | Return Pointer to Memory   |
                                                     +----------------------------+
                                                               |
                                                               v
                                          +-------------------------------------+
                                          | User Request to Free Memory         |
                                          +-------------------------------------+
                                                     |
                                                     v
                                      +-----------------------------+
                                       | Is the pointer valid?     |
                                      +-----------------------------+
                                         | No                      | Yes
                                         v                         v
                                +-------------------+    +----------------------------+
                                | Error Handling    |    | Free Memory (no-op in phase)|
                                | - Output Error    |    +----------------------------+
                                +-------------------+                |
                                         |                           v
                                         v                    +-----------------+
                                        End                   | End             |
                                                              +-----------------+
                              
```

## Implementation of the free list


When implementing a free list for a heap memory manager, the choice between managing the heap as a single free node initially versus splitting it into multiple nodes right away has important implications for efficiency and complexity. Here’s a breakdown of both approaches to help you determine which might be better for your specific use case:

### 1. **Single Free Node (Initial Block) Approach**

**Description**:

- **Initial State**: The entire memory between the base and the program break is managed as a single free node.
- **On Allocation**: When `malloc` is called, this single free node is split into two nodes: one node of the requested block size and another node for the remaining free space.

**Advantages**:

- **Simplicity**: Initially managing memory as a single free node simplifies the setup. You start with one free block and only need to handle splitting when allocation occurs.
- **Reduced Overhead**: Fewer nodes to manage initially means reduced complexity in the early stages of memory management.

**Disadvantages**:

- **Potentially Inefficient Splitting**: If the free space is large, the initial allocation might result in splitting large blocks into many small fragments. This could lead to fragmentation and inefficient use of memory over time.
- **Handling Large Allocations**: If a large block is requested, and the initial single free node is split, the remaining free space might become fragmented and harder to manage.

**Example**:

```c
#define INITIAL_BLOCK_SIZE 64  // Example block size

void initHeap() {
    // Split memory into smaller blocks and create nodes
    // Insert them into the free list
}

void *HmmAlloc(size_t size) {
    // Find suitable free node and allocate
    // Split node if necessary
}
```

### Comparison and Recommendations

**Choosing the Best Approach**:

- **Single Free Node (Initial Block)**:
    - **Best For**: Simpler implementations or when starting with a known large free area.
    - **Considerations**: Suitable for scenarios where you expect relatively straightforward allocation patterns and can handle the potential for fragmentation over time.
- **Multiple Free Nodes (Pre-Split Blocks)**:
    - **Best For**: More complex systems where you want to minimize fragmentation from the start and can handle the complexity of managing multiple nodes.
    - **Considerations**: Ideal for scenarios with varying allocation sizes and when fine-tuning memory management is critical.

**In Practice**:

- Many real-world memory managers, such as those in modern operating systems and libraries, use a combination of both approaches. They might start with a single large block and then progressively manage smaller blocks as needed, incorporating sophisticated techniques to minimize fragmentation and optimize performance.

**Recommendation**:

- **Start Simple**: Begin with the single free node approach to get a working implementation. This is easier to understand and manage initially.
- **Iterate and Optimize**: As your implementation stabilizes, consider evolving towards a more sophisticated approach if you encounter performance or fragmentation issues.

By starting simple and gradually adding complexity based on your needs, you can effectively balance between manageability and performance.

## Implementation of free()

In order to add the block to the (doubly linked) free list, free() uses the bytes of the block itself

→ Get the block length allocated and set by malloc().

→ Construct the free blocks list.

![image](https://github.com/user-attachments/assets/f4e55896-64d9-4edc-a08b-a92aa7fbc0d7)

→ The blocks of the free list will become intermingled with blocks of allocated, in-use memory:

![image](https://github.com/user-attachments/assets/8a74b07b-29ab-4a33-bdf3-7ae557660013)

## Surviving Rules in Dynamic memory Allocation

→ DO NOT touch any memory outside the allocated block range.

→ DO NOT free an allocated block twice.

→ Free with the same pointer returned from malloc. NOT with offset.

→ Free the allocated memory.

### Common Causes of Incorrect Length Modification

1. **Pointer Arithmetic Errors**: Incorrect calculations or manipulations involving pointers can lead to unintended changes in memory locations.
2. **Buffer Overflows**: Writing beyond the allocated memory bounds can corrupt adjacent memory, including metadata that tracks the size of allocated blocks.
3. **Dangling Pointers**: Using pointers after the memory they point to has been freed can lead to unpredictable behavior, including modifying memory block lengths.
4. **Memory Corruption**: Bugs in the program that overwrite memory can inadvertently change the length values of allocated blocks.


## Implementation of malloc()

It first scans the list of memory blocks previously released by free() in order to find one whose size is larger than or equal to its requirements. 
→ Search for the free blocks for a good candidate (first-fit, best-fit, etc.).

If the block is exactly the right size, then it is returned to the caller. 
If it is larger, then it is split, so that a block of the correct size is returned to the caller and a smaller free block is left on the free list.

→ Split the block if its size is larger than needed.

 
If no block on the free list is large enough, then malloc() calls sbrk() to allocate more memory. 

→ Call sbrk() if no free block matching (with larger size).

How free() will know the size of the block?

→ When malloc() allocates the block, it allocates extra bytes to hold an integer containing the size of the block.

![image](https://github.com/user-attachments/assets/1583338c-e058-4aec-bc78-700e57f79728)


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



https://github.com/user-attachments/assets/6d6f3a6d-4479-4000-9498-6ce88c3ac70b



