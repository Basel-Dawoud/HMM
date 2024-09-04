#ifndef HEAP_H
#define HEAP_H

#include <stddef.h>
#include <stdint.h>

// Define the page size
#define PAGE (4096)
#define VHEAP_MAX_SIZE (1024 * 1024 * 1024)
#define META_DATA_SIZE sizeof(fnode)

// Free node structure
typedef struct fnode {
    size_t length;        // Length of the block
    struct fnode *prev;   // Pointer to the previous free node
    struct fnode *next;   // Pointer to the next free node
} fnode;

// Function prototypes
void* sbreak(size_t increment);
void freeListInit(void);
int insertend(void);
void mergeNodes(void);
void* addafternode(fnode* node);
void split(fnode* node, size_t blockSize);
void* firstFit(size_t blockSize);
void* HmmAlloc(size_t blockSize);
void HmmFree(void* ptr);
void* Hmmcalloc(size_t nmemb, size_t size);
void* HmmRealloc(void* ptr, size_t size);
void printFreeList();
// Standard library function wrappers
void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);

#endif // HEAP_H
