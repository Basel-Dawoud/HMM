# Heap Memory Allocator

This repository provides a simple implementation of a heap memory allocator in C using a simulated heap and a free list. The allocator mimics basic functionality of dynamic memory allocation and deallocation.

## Features

- Simulated heap memory management
- Simple free list implementation for tracking free memory blocks
- Basic allocation (`HmmAlloc`) and deallocation (`HmmFree`) functions
- First-fit allocation strategy
- Block splitting and merging to manage memory efficiently

## Installation

To build and run the code, follow these steps:

1. **Clone the Repository:**

   ```sh
   git clone https://github.com/yourusername/heap-allocator.git
   cd heap-allocator
   ```

2. **Compile the Code:**

   Use a C compiler like `gcc` to compile the code. Run the following command in the root directory of the repository:

   ```sh
   gcc -o heap_allocator HMM.c
   ```

3. **Run the Program:**

   Execute the compiled program:

   ```sh
   ./heap_allocator
   ```

## Usage

The primary functions provided are:

- **`void *HmmAlloc(size_t blockSize);`**

  Allocates a block of memory of the specified size. Returns a pointer to the allocated memory or `NULL` if allocation fails.

- **`void HmmFree(void *ptr);`**

  Frees a previously allocated block of memory. The pointer `ptr` should be the same as the one returned by `HmmAlloc`.

### Example

```c
#include "HMM.h"

int main() {
    // Allocate memory
    void *ptr = HmmAlloc(128);
    if (ptr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    // Use allocated memory...

    // Free memory
    HmmFree(ptr);

    return 0;
}
```

## Implementation Details

### Heap Structure

- **Virtual Heap (`vHeap`)**: A static array simulates the heap's memory space.
- **Program Break (`programBreak`)**: Points to the end of the allocated memory.
- **Free List**: Maintains a doubly linked list of free memory blocks.

### Free List Management

- **Initialization**: The `freeListInit` function initializes the free list with one large block at the start of the heap.
- **Adding Nodes**: The `addfnode` function adds a new free node after a given node in the list.
- **Merging Nodes**: The `mergeNodes` function merges adjacent free blocks to consolidate free space.
- **Splitting Nodes**: The `split` function divides a larger block into smaller blocks if it is larger than the requested size.

### Memory Allocation and Deallocation

- **Allocation (`HmmAlloc`)**:
  - Finds a suitable block using the first-fit strategy.
  - Splits the block if it is larger than the requested size.
  - Returns a pointer to the allocated memory.

- **Deallocation (`HmmFree`)**:
  - Inserts the freed block into the free list.
  - Merges adjacent free blocks to avoid fragmentation.
