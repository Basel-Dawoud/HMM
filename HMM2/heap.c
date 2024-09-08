/* HMM.c (Functions & APIs) */

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "heap.h"

/* Declaration of the static array representing the virtual heap */
static size_t* heapBase = NULL; // Pointer to the base of the virtual heap
static size_t* programBreak = NULL; // Pointer representing the current end of the heap

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;

int isHeapFull = 0;
int isFlistAvailable = 0;

/* Adjusts the simulated program break */
void *sbreak(size_t increment) {
    void* oldProgBreak = sbrk(0);   // Get current program break
    if (sbrk(increment) == (void*)-1) {
        return (void*)-1;  // Return error if sbrk fails
    }
    return oldProgBreak;  // Return old end of heap
}

void *HmmAlloc(size_t blockSize) {
    if (Head == NULL) {
        freeListInit();  // Initialize the free list if it is empty
    }

    if (isHeapFull) {
        return NULL;
    }

    // Align block size to be a multiple of 8
    blockSize = (blockSize + 7) & ~7;  // Align the block size to 8 bytes
    size_t totalSizeNeeded = blockSize + META_DATA_SIZE;   // Calculate total size needed including node overhead
    fnode* allocBlock = (fnode*)firstFit(totalSizeNeeded);  // Find a suitable block using first-fit strategy

    if (allocBlock == NULL) {
        size_t pagesNeeded = (totalSizeNeeded + PAGE - 1) / PAGE;  // Calculate pages needed for allocation
        if (sbreak(pagesNeeded * PAGE) == (void*)-1) {  // Attempt to expand the heap
            isHeapFull = 1;
            return NULL; // Allocation failed
        }
        int failed = insertend(pagesNeeded);  // Insert the new block into the free list
        if (failed == -1) return NULL;
        allocBlock = (fnode*)firstFit(totalSizeNeeded);
        if (allocBlock == NULL) return NULL;  // Handle failure if no block was found
    }
    
    // Remove block from free list
    if (allocBlock == Head) {
        Head = Head->next;  // Update head if the allocated block was the head
        if (Head) {
            Head->prev = NULL;  // Update previous pointer of the new head
        }
    } else if (allocBlock == Tail) {
        Tail = Tail->prev;  // Update tail if the allocated block was the tail
        if (Tail) {
            Tail->next = NULL;  // Update next pointer of the new tail
        }
    } else {
        if (allocBlock->prev) {
            allocBlock->prev->next = allocBlock->next;  // Update previous node's next pointer
        }
        if (allocBlock->next) {
            allocBlock->next->prev = allocBlock->prev;  // Update next node's previous pointer
        }
    }

    allocBlock->next = NULL;  // Set next pointer of allocated block to NULL
    allocBlock->prev = NULL;  // Set previous pointer of allocated block to NULL

    allocBlock = (fnode*)((char*)allocBlock + META_DATA_SIZE);  // Adjust the pointer to point to the start of the usable memory
    return (void*)allocBlock;  // Return the pointer to the allocated memory
}

/* Initializes the free list */
void freeListInit(void) {
    // Initialize the heap and the free list
    size_t initialHeapSize = 2 * PAGE;
    if (heapBase == NULL) {
        heapBase = (size_t*)sbreak(initialHeapSize);
        if (heapBase == (void*)-1) {
            isHeapFull = -1;
            return;
        }
        programBreak = heapBase;
    }

    Head = (fnode*)programBreak;  // Set the head of the list to the start of the heap
    Head->length = PAGE / 8;  // Set the initial block size
    Head->prev = NULL;  // Set the previous pointer of the head to NULL
    Head->next = NULL;  // Set the next pointer of the head to NULL
    Tail = Head;  // Set the tail of the list to the head
    insertend(1);
    isFlistAvailable = 1;
}

/* Adds a new free node after the given node */
void* addafternode(fnode* node) {
    size_t* pose = (size_t*)((char*)node + node->length);  // Calculate the position for the new node
    fnode* newNode = (fnode*)pose;  // Create new node at calculated position
    newNode->next = node->next;  // Set the next pointer of the new node
    newNode->prev = node;  // Set the previous pointer of the new node

    if (node->next) { // Ensure node->next is not NULL
        node->next->prev = newNode;  // Update the previous pointer of the next node
    }
    node->next = newNode;  // Set the next pointer of the given node to the new node
    return newNode;
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize) {
    size_t oldlength = node->length;  // Store the old length of the node
    size_t minBlockSize = META_DATA_SIZE + 8; // Minimum block size to split

    if ((oldlength - blockSize) >= minBlockSize) {
        // Adjust the current node's length
        node->length = blockSize;
        // Create a new node with the remaining space
        fnode* newNode = (fnode*)((char*)node + blockSize);
        newNode->length = oldlength - blockSize;
        newNode->next = node->next;
        newNode->prev = node;

        if (node->next) {
            node->next->prev = newNode;
        }
        node->next = newNode;

        // Update the tail if the new node is the new tail
        if (node == Tail) {
            Tail = newNode;
        }
    }
}

/* Finding a free node that fits the requested block size using the first-fit strategy */
void *firstFit(size_t blockSize) {
    assert(Head != NULL);
    fnode* curr = Head;
    while (curr) {
        if (curr->length >= blockSize) {
            if (curr != Head && curr != Tail) {
                split(curr, blockSize);  // Split the node if it is not the head or tail
            }
            return curr;  // Return the node that fits the requested block size
        }
        curr = curr->next;
    }
    return NULL;  // Return NULL if no suitable node is found
}

/* Adds a new free node after the Tail */
int insertend(int pagesNeeded) {
    // Ensure that the new node does not exceed the program Break edge

    void* cbp = sbrk(pagesNeeded * PAGE);
    if (cbp == (void*)-1) return -1;

    fnode* newNode = (fnode*)((char*)Tail + Tail->length);  // Position the new node after the tail
    newNode->prev = Tail;  // Set the previous pointer of the new node to the old tail
    Tail->next = newNode;  // Set the next pointer of the old tail to the new node
    Tail = newNode;  // Update the tail to be the new node
    Tail->next = NULL;  // Set the next pointer of the new tail to NULL
    /* Set the length of the new node */
    Tail->length = (size_t)((char*)cbp - (char*)Tail);  // Set the length of the new Tail-node

    mergeNodes();
    return 0;  // Success
}

void HmmFree(void *ptr) {
    if (ptr == NULL) {
        return;  // Do nothing if the pointer is NULL
    }

    if (!isFlistAvailable) return;

    fnode* blockToFree = (fnode*)((char*)ptr - sizeof(fnode));  // Get the free node from the pointer

    // Insert block into the free list
    blockToFree->next = Head;  // Set the next pointer of the block to the current head
    blockToFree->prev = NULL;  // Set the previous pointer of the block to NULL
    if (Head) {
        Head->prev = blockToFree;  // Update the previous pointer of the old head
    }
    Head = blockToFree;  // Update the head to the new block

    mergeNodes();  // Merge adjacent free nodes
}

/* This function handles merging of adjacent free nodes */
void mergeNodes(void) {
    if (!Head || !(Head->next)) {
        return;  // Nothing to merge
    }

    fnode* curr = Head;
    while (curr && curr->next) {
        if ((char*)curr + curr->length == (char*)curr->next) {
            // Merge adjacent nodes
            curr->length += curr->next->length;
            curr->next = curr->next->next;
            if (curr->next) {
                curr->next->prev = curr;
            }
            else {
                Tail = curr;  // Update Tail if the last node was merged
            }
        } else {
            curr = curr->next;  // Move to the next node
        }
    }
}

void *HmmRealloc(void *ptr, size_t blockSize) {
    if (ptr == NULL) {
        return HmmAlloc(blockSize);  // Allocate new block if pointer is NULL
    }

    if (blockSize == 0) {
        HmmFree(ptr);  // Free the block if the requested size is zero
        return NULL;
    }

    fnode* oldBlock = (fnode*)((char*)ptr - sizeof(fnode));  // Get the old block
    size_t oldSize = oldBlock->length;

    if (blockSize <= oldSize) {
        // Block is large enough; split if there is excess space
        split(oldBlock, blockSize);
        return ptr;
    }

    // Allocate new block
    void* newPtr = HmmAlloc(blockSize);
    if (newPtr == NULL) {
        return NULL;  // Allocation failed
    }

    // Copy data from old block to new block
    memcpy(newPtr, ptr, oldSize);

    // Free the old block
    HmmFree(ptr);

    return newPtr;
}

// Wrapper functions to replace the libc ABIS...

void* malloc(size_t size) {
    return HmmAlloc(size);
}

void free(void* ptr) {
    HmmFree(ptr);
}

void* calloc(size_t nmemb, size_t size) {
    return HmmCalloc(nmemb, size);
}

void* realloc(void* ptr, size_t size) {
    return HmmRealloc(ptr, size);
}
