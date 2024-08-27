#include <stddef.h>
#include <stdio.h>
#include <unistd.h>  // For sbrk()
#include <string.h>  // For memset(), memcpy()
#include "heap.h"
#include <stdlib.h>

/* Declaration of the static array representing the virtual heap */
static char *heapBase = NULL;
static char *programBreak = NULL;

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;

/* Adjusts the simulated program break */
void *sbreak(intptr_t increment) {
    void *oldBreak = sbrk(0);  // Get the current break point
    if (sbrk(increment) == (void *)-1) {
        return (void *)-1;  // sbrk failed
    }
    return oldBreak;
}

/* Initializes the free list */
void freeListInit(void) {
    heapBase = (char *)sbreak(PAGE); // At the start of the Heap
    if (heapBase == (void *)-1) {
        perror("sbrk failed");
        exit(EXIT_FAILURE);
    }
    programBreak = heapBase;
    Head = (fnode *)heapBase;
    Head->length = (size_t)sbreak(0) - (size_t)Head;
    if (Head == (void *)-1) {
        perror("sbrk failed");
        exit(EXIT_FAILURE);
    }
    Head->prev = NULL;
    Head->next = NULL;
    Tail = Head;
}

/* Adds a new free node after the given node */
void* addfnode(fnode* node) {
    fnode* newNode = (fnode *)((char *)node + node->length);

    newNode->next = node->next;
    newNode->prev = node;
    
    if (node->next) {
        node->next->prev = newNode;    
    } else {
        Tail = newNode; // Update Tail if it's the last node
    }
    node->next = newNode;

    if (newNode->next) {
        newNode->length = (size_t)((char *)node->next - (char *)newNode);
    } else {
        newNode->length = (size_t)((char *)sbreak(0) - (char *)newNode);
    }

    return newNode;
}

/* This function handles merging of adjacent free nodes */
void mergeNodes(void) {
    fnode* curr = Head;
    
    while (curr && curr->next) {
        if ((char *)curr + curr->length == (char *)curr->next) {
            // Merge with next node
            curr->length += curr->next->length;
            fnode* nextNode = curr->next->next;
            
            if (nextNode) {
                nextNode->prev = curr;
            } else {
                Tail = curr;
            }
            
            curr->next = nextNode;
        } else {
            curr = curr->next;
        }
    }
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize) {
    if (Head == NULL) { // No linked list yet
        freeListInit();
        return;
    }

    if (node->length > blockSize + META_DATA_SIZE) {
        fnode* newNode = (fnode *)((char *)node + blockSize + META_DATA_SIZE);
        newNode->length = node->length - blockSize - META_DATA_SIZE;
        newNode->next = node->next;
        newNode->prev = node;
        if (node->next) {
            node->next->prev = newNode;
        } else {
            Tail = newNode;
        }
        node->next = newNode;
        node->length = blockSize;
    }
}

/* Finds a free node that fits the requested block size using the first-fit strategy */
void *firstFit(size_t blockSize) {
    if (blockSize < META_DATA_SIZE) {
        blockSize = META_DATA_SIZE;
    }

    for (fnode* curr = Head; curr != NULL; curr = curr->next) {
        if (curr->length >= blockSize) {
            split(curr, blockSize);
            return (void *)((char *)curr + META_DATA_SIZE);
        }
    }

    return NULL;
}

void *HmmAlloc(size_t blockSize) {
    if (blockSize < META_DATA_SIZE) {
        blockSize = META_DATA_SIZE;
    }

    blockSize = (blockSize + 7) & ~7; // Align block size to be a multiple of 8

    if (Head == NULL) {
        freeListInit();
    }

    size_t totalSizeNeeded = blockSize + META_DATA_SIZE;
    fnode* allocBlock = (fnode *)firstFit(totalSizeNeeded);

    if (allocBlock == NULL) {
        size_t pagesNeeded = (totalSizeNeeded + PAGE - 1) / PAGE;
        if (sbreak(pagesNeeded * PAGE) == (void *)-1) {
            return NULL;
        }
        addfnode(Tail);
        allocBlock = (fnode *)firstFit(totalSizeNeeded);
    }

    if (allocBlock == NULL) {
        return NULL; // Allocation failed
    }
   
    if (allocBlock == Head) {
        Head = allocBlock->next;
        if (Head) {
            Head->prev = NULL;
        }
    } else if (allocBlock == Tail) {
        Tail = allocBlock->prev;
        if (Tail) {
            Tail->next = NULL;
        }
    } else {
        allocBlock->prev->next = allocBlock->next;
        if (allocBlock->next) {
            allocBlock->next->prev = allocBlock->prev;
        }
    }

    allocBlock->next = NULL;
    allocBlock->prev = NULL;

    return (void *)((char *)allocBlock + META_DATA_SIZE);
}

void HmmFree(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    fnode* blockToFree = (fnode *)((char *)ptr - META_DATA_SIZE);

    if (Head == NULL) {
        freeListInit();
    }

    blockToFree->next = Head;
    blockToFree->prev = NULL;
    if (Head) {
        Head->prev = blockToFree;
    }
    Head = blockToFree;

    mergeNodes();
}

/* libc memory management functions */

void *malloc(size_t size) {
    return HmmAlloc(size);
}

void free(void *ptr) {
    HmmFree(ptr);
}

void *calloc(size_t num, size_t size) {
    size_t totalSize = num * size;
    void *ptr = HmmAlloc(totalSize);
    if (ptr) {
        memset(ptr, 0, totalSize); // Zero out the allocated memory
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return HmmAlloc(size);
    }

    if (size == 0) {
        HmmFree(ptr);
        return NULL;
    }

    fnode* oldBlock = (fnode *)((char *)ptr - META_DATA_SIZE);
    size_t oldSize = oldBlock->length;

    void *newPtr = HmmAlloc(size);
    if (newPtr) {
        memcpy(newPtr, ptr, oldSize < size ? oldSize : size);
        HmmFree(ptr);
    }

    return newPtr;
}

