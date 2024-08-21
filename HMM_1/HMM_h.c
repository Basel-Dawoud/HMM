#ifndef HMM_H
#define HMM_H

#include <stddef.h>

// Define constants
#define VHEAP_MAX_SIZE 1024 * 1024 // 1 MB for example

// Function prototypes for the Heap Memory Manager
void *HmmAlloc(size_t size);
void HmmFree(void *ptr);

// Function prototypes for the linked list operations
void insertatbegin(int data);
void insertatend(int data);
void insertafternode(struct node *list, int data);
void deleteatbegin();
void deleteatend();
void deletenode(int key);
void printList();

// Function prototypes for managing the free list
void freeListInit(void);
void updateFreeList(struct fnode *lastNode);
void *firstFit(void);
void printFreeList(void);

// Define a structure for the linked list nodes
struct node {
    int data;
    struct node *next;
};

// Define a structure for free list nodes
struct fnode {
    size_t size;
    struct fnode *next;
};

// External variables for the Heap Memory Manager
extern void *heapBase;
extern void *Tail;
extern void *programBreak;
extern void *currNodePtr;
extern int available;

#endif // HMM_H
