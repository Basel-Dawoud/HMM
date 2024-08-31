#ifndef HMM_APIS_H
#define HMM_APIS_H

#include <stddef.h>
#include <stdint.h>


/* Define heap size and page size */
#define VHEAP_MAX_SIZE (1024*1024*1024) /* Total size of the virtual heap */
#define PAGE (4096) /* 4KB page size for demonstration */
#define META_DATA_SIZE sizeof(fnode) /* Size of metadata for each free node */


/* Structure of a free node in the free list */
typedef struct freeNode {
    size_t length;         /* Size of the free block */
    struct freeNode* prev; /* Pointer to the previous free block */
    struct freeNode* next; /* Pointer to the next free block */
} fnode;

void *sbreak(intptr_t increment);

void freeListInit(void);

void* addfnode(fnode* node);

void mergeNodes(void);

void split(fnode* node, size_t blockSize);

void *firstFit(size_t blockSize);

void *HmmAlloc(size_t blockSize);

void HmmFree(void *ptr);

// Print the free list to check initial state
void printFreeList();

void insertend(size_t neweSize);

void splitAllocNode(fnode* node, size_t blockSize);

void *calloc(size_t nmemb, size_t size);

void *realloc(void *ptr, size_t size);

#endif /* HMM_APIS_H */
