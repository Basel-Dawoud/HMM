#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define PAGE 4096  // Define page size (usually 4KB)
#define VHEAP_MAX_SIZE (1024 * PAGE)  // Define the maximum size of heap for testing

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;

/* Initialize the heap space using sbrk() */
void* heapBase = NULL;  // Base address of heap
void* programBreak = NULL;  // Pointer representing the current end of the heap

void* initializeHeap() {
    if (heapBase == NULL) {
        heapBase = sbrk(VHEAP_MAX_SIZE);
        if (heapBase == (void*)-1) {
            perror("sbrk");
            return NULL; // Error
        }
        programBreak = heapBase;
    }
    return heapBase;
}

/* Adjust the simulated program break */
void *sbreak(intptr_t increment) {
    void* oldProgBreak = programBreak;
    void* newProgBreak = sbrk(increment);
    if (newProgBreak == (void*)-1) {
        return (void *)-1; // Return error if sbrk fails
    }
    programBreak = newProgBreak;
    return oldProgBreak;
}

/* Initializes the free list */
void freeListInit(void) {
    if (initializeHeap() == NULL) {
        return;  // Initialization failed
    }
    Head = (fnode*)programBreak;
    Head->length = (size_t)sbrk(0) - (size_t)Head; // Set the initial block size
    Head->prev = NULL;
    Head->next = NULL;
    Tail = Head;
}

/* Adds a new free node after the Tail */
void insertend(size_t neweSize) {
    assert(((char*)Tail + Tail->length) < (char*)programBreak);  // Ensure no overflow
    fnode* newNode = (fnode*)((char*)Tail + Tail->length);
    newNode->prev = Tail;
    Tail->next = newNode;
    Tail = newNode;
    Tail->next = NULL;
    Tail->length = neweSize;
    mergeNodes();
}

/* This function handles merging of adjacent free nodes */
void mergeNodes(void) {
    if (!Head || !(Head->next)) {
        return; // Nothing to merge
    }

    fnode* curr = Head;
    while (curr && curr->next) {
        if ((char*)curr + curr->length == (char*)curr->next) {
            curr->length += curr->next->length;
            curr->next = curr->next->next;
            if (curr->next) {
                curr->next->prev = curr;
            } else {
                Tail = curr;
            }
        } else {
            curr = curr->next;
        }
    }
}

/* Adds a new free node after the given node */
void* addafternode(fnode* node) {
    char* pose = (char*)node + node->length;
    fnode* newNode = (fnode*)pose;
    newNode->next = node->next;
    newNode->prev = node;

    if (node->next) {
        node->next->prev = newNode;
    }
    node->next = newNode;

    return newNode;
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize) {
    if (Head == NULL) {
        freeListInit();
        return;
    }

    size_t oldlength = node->length;

    if (blockSize >= oldlength) {
        return;
    }

    node->length = blockSize;
    fnode* newNode = (fnode*)addafternode(node);
    newNode->length = oldlength - blockSize;
}

/* Finding a free node that fits the requested block size using the first-fit strategy */
void *firstFit(size_t blockSize) {
    fnode* curr = NULL;
    for (curr = Head; curr != NULL; curr = curr->next) {
        if (curr->length >= blockSize) {
            if (curr != Head && curr != Tail) {
                split(curr, blockSize);
            }
            return curr;
        }
    }
    return NULL;
}

void *HmmAlloc(size_t blockSize) {
    if (Head == NULL) {
        freeListInit();
    }

    blockSize = (blockSize + 7) & ~7;  // Align block size to 8 bytes
    size_t totalSizeNeeded = blockSize + sizeof(fnode);
    fnode* allocBlock = (fnode*)firstFit(totalSizeNeeded);

    if (allocBlock == NULL) {
        size_t pagesNeeded = (totalSizeNeeded + PAGE - 1) / PAGE;
        if (sbreak(pagesNeeded * PAGE) == (void*)-1) {
            return NULL;
        }
        insertend(totalSizeNeeded);
        allocBlock = Tail;
        assert(allocBlock != NULL);
        assert(allocBlock->next == NULL);
    }

    if (allocBlock == Head) {
        Head = Head->next;
        if (Head) {
            Head->prev = NULL;
        }
    } else if (allocBlock == Tail) {
        Tail = Tail->prev;
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

    allocBlock = (fnode*)((char*)allocBlock + sizeof(fnode));
    return (void*)allocBlock;
}

void HmmFree(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    if (Head == NULL) {
        freeListInit();
    }

    fnode* blockToFree = (fnode*)((char*)ptr - sizeof(fnode));

    blockToFree->next = Head;
    blockToFree->prev = NULL;
    assert(Head != NULL);
    Head->prev = blockToFree;
    Head = blockToFree;

    mergeNodes();
}

void *calloc(size_t nmemb, size_t size) {
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return NULL;  // Overflow occurred
    }

    size_t totalSize = nmemb * size;

    void *ptr = HmmAlloc(totalSize);
    if (ptr == NULL) {
        return NULL;  // Allocation failed
    }

    memset(ptr, 0, totalSize);
    return ptr;
}

void splitAllocNode(fnode* node, size_t blockSize) {
    size_t oldSize = node->length;
    node->length = blockSize + sizeof(fnode);
    fnode* blockToFree = (fnode*)((char*)node + node->length);
    blockToFree->length = oldSize - node->length;
    HmmFree((char*)blockToFree + sizeof(fnode));
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return HmmAlloc(size);
    }

    if (size == 0) {
        HmmFree(ptr);
        return NULL;
    }

    size = (size + 7) & ~7;

    fnode* node = (fnode*)((char*)ptr - sizeof(fnode));
    size_t oldSize = node->length - sizeof(fnode);

    if (size < oldSize) {
        splitAllocNode(node, size);
        return ptr;
    }

    if (size > oldSize) {
        size_t additional_size = size - oldSize;

        for (fnode* curr = Head; curr != NULL; curr = curr->next) {
            if (curr->length >= additional_size) {
                if ((char*)curr == (char*)node + node->length) {
                    if (curr->length > (additional_size + sizeof(fnode))) {
                        split(curr, additional_size);
                        return ptr;
                    } else if (curr->length == (additional_size + sizeof(fnode))) {
                        return ptr;
                    }
                }
            }
        }

        fnode* newBlock = (fnode*)firstFit(size + sizeof(fnode));
        if (newBlock == NULL) {
            return NULL;
        }

        void* dest = (char*)newBlock + sizeof(fnode);
        memcpy(dest, ptr, oldSize);

        HmmFree(ptr);
        return dest;
    }

    return ptr;
}
