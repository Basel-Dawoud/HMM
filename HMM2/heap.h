/*HMM.c (Functions & APIs)*/

#include <stddef.h>
#include <stdio.h>
#include "heap.h"
#include <assert.h>
#include <string.h>
#include <limits.h>

/* Declaration of the static array representing the virtual heap */
static int vHeap[VHEAP_MAX_SIZE];  // Static array simulating the heap space
static int *heapBase = vHeap;      // Pointer to the base of the virtual heap
static int *programBreak = vHeap; // Pointer representing the current end of the heap

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;

/* Adjusts the simulated program break */
void *sbreak(intptr_t increment) {
    int* oldProgBreak = programBreak;   // Save the current end of heap
    int* newProgBreak = programBreak + increment; // Calculate new end of heap
    if (newProgBreak > (heapBase + VHEAP_MAX_SIZE) || newProgBreak < heapBase) {
        printf("SIGSEGV: Program break exceeded limits\n");    
        return (void *) -1; // Return error if new break exceeds limits
    }
    programBreak = newProgBreak;  // Update the end of heap
    return oldProgBreak;  // Return old end of heap
}

/* Initializes the free list */
void freeListInit(void){
    Head  = (fnode*)sbreak(PAGE); /* Set the head of the list to the start of the heap */
    Head->length = (size_t)sbreak(0) - (size_t)Head; // Set the initial block size
    Head->prev = NULL;  // Set the previous pointer of the head to NULL
    Head->next = NULL;  // Set the next pointer of the head to NULL
    Tail = Head;  // Set the tail of the list to the head
}		

/* Adds a new free node after the Tail */
void insertend(size_t neweSize){
    assert(((int*)Tail + Tail->length) < programBreak);  // Ensure that the new node does not exceed the heap boundary
    /* Calculate position for new node */
    fnode* newNode = (fnode*)((int*)Tail + Tail->length);  // Position the new node after the tail
    newNode->prev = Tail;  // Set the previous pointer of the new node to the old tail
    Tail->next = newNode;  // Set the next pointer of the old tail to the new node
    Tail = newNode;  // Update the tail to be the new node
    Tail->next = NULL;  // Set the next pointer of the new tail to NULL
    /* Set the length of the new node */
    Tail->length = neweSize;  // Set the length of the new node
    mergeNodes();
}

/* This function handles merging of adjacent free nodes */
void mergeNodes(void) {
    // Check if the list is empty or has only one node
    if (!Head || !(Head->next)) {
        return;  // Nothing to merge
    }
    
    fnode* curr = Head;  // Start from the head of the list
    while (curr && curr->next) {
        // If current node and next node are adjacent
        if ((char*)curr + curr->length == (char*)curr->next) {
            // Merge current and next nodes
            curr->length += curr->next->length;  // Increase the length of the current node
            curr->next = curr->next->next;  // Skip the next node
            
            if (curr->next) {
                curr->next->prev = curr;  // Update the previous pointer of the new next node
            } else {
                // Update Tail if we merged the last node
                Tail = curr;  // Update the tail to the current node if the last node was merged
            }
        } else {
            // Move to the next node
            curr = curr->next;  // Move to the next node
        }
    }
}

/* Adds a new free node after the given node */
void* addafternode(fnode* node){
    char* pose = (char*)node + node->length;  // Calculate the position for the new node
    fnode* newNode = (fnode*)pose;  // Create new node at calculated position
    newNode->next = node->next;  // Set the next pointer of the new node
    newNode->prev = node;  // Set the previous pointer of the new node
    
    if (node->next) { // Ensure node->next is not NULL
        node->next->prev = newNode;  // Update the previous pointer of the next node
    }
    node->next = newNode;  // Set the next pointer of the given node to the new node
    
    /* Return the new node */
    return newNode;
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize){
    if (Head == NULL) { /* No linked list yet */
        freeListInit();  // Initialize the free list if it is empty
        return;
    }
    
    size_t oldlength = node->length;  // Store the old length of the node
    
    if (blockSize >= oldlength) {
        // Cannot split; blockSize is too large or exactly matches node's length
        return;  // Return without splitting if the block size is too large
    }
    
    node->length = blockSize;  // Set the length of the current node to the requested block size
    fnode* newNode = (fnode*)addafternode(node);  // Add a new node after the current node
    newNode->length = oldlength - blockSize;  // Set the length of the new node to the remaining space
}

/* Finding a free node that fits the requested block size using the first-fit strategy */
void *firstFit(size_t blockSize){

    fnode* curr = NULL;  // Pointer to traverse the list
    for(curr = Head; curr != NULL ; curr = curr->next){
        if(curr->length >= blockSize){
           if(curr != Head && curr != Tail){
               split(curr,blockSize);  // Split the node if it is not the head or tail
           }
           return curr;  // Return the node that fits the requested block size
        }
    }
    return NULL;  // Return NULL if no suitable node is found
}

void *HmmAlloc(size_t blockSize) {
    if (Head == NULL) {
        freeListInit();  // Initialize the free list if it is empty
    }

    // Align block size to be a multiple of 8
    blockSize = (blockSize + 7) & ~7;  // Align the block size to 8 bytes
    size_t totalSizeNeeded = blockSize + sizeof(fnode);   // Calculate total size needed including node overhead
    fnode* allocBlock = (fnode*)firstFit(totalSizeNeeded);  // Find a suitable block using first-fit strategy
    
    if (allocBlock == NULL) {
        size_t pagesNeeded = (totalSizeNeeded + PAGE - 1) / PAGE;  // Calculate pages needed for allocation
        if (sbreak(pagesNeeded * PAGE) == (void*)-1) {  // Attempt to expand the heap
            return NULL; // Allocation failed
        }
        insertend(totalSizeNeeded);  // Insert the new block into the free list
        allocBlock = Tail;  // Set the allocated block to the new tail
        assert(allocBlock != NULL);  // Assert that the allocated block is not NULL
        assert(allocBlock->next == NULL);  // Assert that the next pointer of the new tail is NULL
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
        allocBlock->prev->next = allocBlock->next;  // Update previous node's next pointer
        if (allocBlock->next) {
            allocBlock->next->prev = allocBlock->prev;  // Update next node's previous pointer
        }
    }

    allocBlock->next = NULL;  // Set next pointer of allocated block to NULL
    allocBlock->prev = NULL;  // Set previous pointer of allocated block to NULL

    allocBlock = (fnode*)((int*)allocBlock + sizeof(fnode));  // Adjust the pointer to point to the start of the usable memory
    return (void*)allocBlock;  // Return the pointer to the allocated memory
}

void HmmFree(void *ptr) {
    if (ptr == NULL) {
        return;  // Do nothing if the pointer is NULL
    }

    if (Head == NULL){
        freeListInit();  // Initialize the free list if it is empty
    }
    
    fnode* blockToFree = (fnode*)((int*)ptr - sizeof(fnode));  // Get the free node from the pointer

    // Insert block into the free list
    blockToFree->next = Head;  // Set the next pointer of the block to the current head
    blockToFree->prev = NULL;  // Set the previous pointer of the block to NULL
    assert(Head != NULL);  // Assert that the head is not NULL
    Head->prev = blockToFree;  // Update the previous pointer of the old head
    Head = blockToFree;  // Update the head to the new block

    mergeNodes();  // Merge adjacent free nodes
}

void splitAllocNode(fnode* node, size_t blockSize){
    size_t oldSize = node->length;
    
    /* Update the length of the node to the new block size */
    node->length = blockSize + sizeof(fnode);
    
    /* Free the remaining part of the block */
    fnode* blockToFree = (fnode*)((char*)node + node->length);
    blockToFree->length = oldSize - node->length;  
    HmmFree((char*)blockToFree + sizeof(fnode));
}


void *calloc(size_t nmemb, size_t size) {
    // Check for overflow in nmemb * size
    if (nmemb != 0 && size > SIZE_MAX / nmemb) {
        return NULL; // Overflow occurred
    }

    size_t totalSize = nmemb * size;

    // Allocate memory
    void *ptr = HmmAlloc(totalSize);
    if (ptr == NULL) {
        return NULL; // Allocation failed
    }

    // Initialize allocated memory to zero
    memset(ptr, 0, totalSize);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    // Case 1: If ptr is NULL, allocate a new block of memory
    if (ptr == NULL) {
        return HmmAlloc(size);
    }
    
    // Case 2: If size is 0, free the memory and return NULL
    if (size == 0) {
        HmmFree(ptr);
        return NULL;
    }

    // Align block size to be a multiple of 8
    size = (size + 7) & ~7;

    // Determine the original node associated with the pointer
    fnode* node = (fnode*)((char*)ptr - sizeof(fnode));
    size_t oldSize = node->length - sizeof(fnode);

    // Case 3: If the new size is smaller, shrink the block
    if (size < oldSize) {
        splitAllocNode(node, size); // Shrink the block and free the remaining part
        return ptr;
    }    

    // Case 4: If the new size is larger, attempt to expand the block
    if (size > oldSize) {
        size_t additional_size = size - oldSize;

        // Try to find a free block that can be merged with the current block
        for (fnode* curr = Head; curr != NULL; curr = curr->next) {
            // Check if current node can satisfy the additional size required
            if (curr->length >= additional_size) {
                // Ensure the current block is after the original node
                if ((char*)curr == (char*)node + node->length) {
                    // Check if the block can be split
                    if (curr->length > additional_size + sizeof(fnode)) {
                        split(curr, additional_size);
                        return ptr;
                    } else if (curr->length == additional_size + sizeof(fnode)) {
                        // The block is exactly the required size
                        return ptr;
                    }
                }
            }
        }

        // No suitable block found, allocate new memory and copy data
        fnode* newBlock = firstFit(size + sizeof(fnode));
        if (newBlock == NULL) {
            return NULL; // No suitable node found
        }

        // Copy data from old (ptr) block to new block
        void* dest = (char*)newBlock + sizeof(fnode);
        memcpy(dest, ptr, oldSize);
        
        // Free the old block after copying the data
        HmmFree(ptr);
        return dest;
    }
    
    // Case 5: The new size is the same as the old size, no changes needed
    return ptr;
}
