#ifndef HMM_APIS_H
#define HMM_APIS_H

#include <stddef.h>
#include <stdint.h>

#define VHEAP_MAX_SIZE (1024 * 1024 * 1024) /* 1GB */
#define PAGE 4096 /* 4KB page size for demonstration */
#define META_DATA_SIZE sizeof(size_t)

/* Structure of a free node in the free list */
typedef struct freeNode {
    size_t length;         /* Size of the free block */
    struct freeNode* prev; /* Pointer to the previous free block */
    struct freeNode* next; /* Pointer to the next free block */
} fnode;

void *sbreak(intptr_t increment);

void freeListInit(void);

void* addfnode(fnode* node);

void *mergeNodes(void);

void split(fnode* node, size_t blockSize);

void *firstFit(size_t blockSize);

void *HmmAlloc(size_t blockSize);

void HmmFree(void *ptr);

#endif /* HMM_APIS_H */
