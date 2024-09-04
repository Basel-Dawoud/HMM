/*HMM.c (Functions & APIs)*/

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include "hmm.h"


/* Declaration of the static array representing the virtual heap */
static size_t vHeap[VHEAP_MAX_SIZE];  // Static array simulating the heap space
static size_t* heapBase = vHeap;      // Pointer to the base of the virtual heap
static size_t* programBreak = vHeap; // Pointer representing the current end of the heap

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;
int isHeapFull = 0;
int isFlistAvailable = 0;
long long abp = 0;
long long ac = 0;
long long af = 0;
long long as = 0;
long long ainsert =0;
long long mergeCount= 0;

/* Adjusts the simulated program break */
void *sbreak(size_t increment) {
    size_t* oldProgBreak = programBreak;   // Save the current end of heap
    size_t* newProgBreak = (size_t*)((char*)programBreak + increment); // Calculate new end of heap
    if (newProgBreak > ((size_t*)((char*)heapBase + VHEAP_MAX_SIZE)) || newProgBreak < heapBase) {
        printf("WARNING!!: Program break exceeded limits\n");    
        return (void *) -1; // Return error if new break exceeds limits
    }
    programBreak = newProgBreak;  // Update the end of heap
    abp++;
    return oldProgBreak;  // Return old end of heap
}


void *HmmAlloc(size_t blockSize) {

    if (Head == NULL) {
        freeListInit();  // Initialize the free list if it is empty
    }

    if(isHeapFull){
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
        int failed = insertend();  // Insert the new block into the free list
        if (failed == -1) return NULL;
        allocBlock = (fnode*)firstFit(totalSizeNeeded);
	if(allocBlock == NULL) return NULL;
        assert(allocBlock == Tail);  // Assert that the next pointer of the new tail is NULL
        assert(allocBlock != NULL);  // Assert that the allocated block is not NULL
        assert(allocBlock->next == NULL);  // Assert that the next pointer of the new tail is NULL
    }
    
    // Remove block from free list
    if (allocBlock == Head) {	
        if (Head && Head->next) {
            Head = Head->next;  // Update head if the allocated block was the head
            Head->prev = NULL;  // Update previous pointer of the new head
        } else{
            return NULL;
        }
    } else if (allocBlock == Tail) {
        //Tail = Tail->prev;  // Update tail if the allocated block was the tail
        if(insertend() == -1) return  NULL;
        if (Tail) {
            Tail->prev = allocBlock->prev;
            Tail->next = NULL;  // Update next pointer of the new tail
        }
    }
    if(allocBlock->prev)
    allocBlock->prev->next = allocBlock->next;  // Update previous node's next pointer
    if (allocBlock->next) {
         allocBlock->next->prev = allocBlock->prev;  // Update next node's previous pointer
    }


    allocBlock->next = NULL;  // Set next pointer of allocated block to NULL
    allocBlock->prev = NULL;  // Set previous pointer of allocated block to NULL

    allocBlock = (fnode*)((char*)allocBlock + META_DATA_SIZE);  // Adjust the pointer to point to the start of the usable memory
    ac++;
    return (void*)allocBlock;  // Return the pointer to the allocated memory
}



/* Initializes the free list */
void freeListInit(void){
	
    Head  = (fnode*)sbreak(2*PAGE); /* Set the head of the list to the start of the heap */
    if((int*)Head == (int*)-1){
        isHeapFull = -1;
        return;
    }
    Head->length = PAGE/8; // Set the initial block size
    Head->prev = NULL;  // Set the previous pointer of the head to NULL
    Head->next = NULL;  // Set the next pointer of the head to NULL
    Tail = Head;  // Set the tail of the list to the head
    insertend();
    isFlistAvailable = 1;
}		

/* Adds a new free node after the given node */
void* addafternode(fnode* node){
    size_t* pose = (size_t*)((char*)node + node->length);  // Calculate the position for the new node
    fnode* newNode = (fnode*)pose;  // Create new node at calculated position
    newNode->next = node->next;  // Set the next pointer of the new node
    newNode->prev = node;  // Set the previous pointer of the new node
    
    if (node->next) { // Ensure node->next is not NULL
        node->next->prev = newNode;  // Update the previous pointer of the next node
    }
    node->next = newNode;  // Set the next pointer of the given node to the new node
    af++;
    /* Return the new node */
    return newNode;
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize){    
    size_t oldlength = node->length;  // Store the old length of the node
    if((oldlength - blockSize) <= 24) return;
    if (blockSize >= oldlength || blockSize <= META_DATA_SIZE) {
        // Cannot split; blockSize is too large or exactly matches node's length
        return;  // Return without splitting if the block size is too large
    }
    
    node->length = blockSize;  // Set the length of the current node to the requested block size
    fnode* newNode = (fnode*)addafternode(node);  // Add a new node after the current node
    newNode->length = oldlength - blockSize;  // Set the length of the new node to the remaining space
    as++;
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


/* Adds a new free node after the Tail */
int insertend(void){
    // Ensure that the new node does not exceed the program Break edge

    int* cbp = (int*)sbreak(PAGE);
    
    // Ensure that the new node does not exceed the program Break edge
    if(cbp == (int*)-1) return -1;
    
    fnode* newNode = (fnode*)((char*)Tail + Tail->length);  // Position the new node after the tail
    newNode->prev = Tail;  // Set the previous pointer of the new node to the old tail
    Tail->next = newNode;  // Set the next pointer of the old tail to the new node
    Tail = newNode;  // Update the tail to be the new node
    Tail->next = NULL;  // Set the next pointer of the new tail to NULL
    /* Set the length of the new node */
    Tail->length = (size_t)((char*)cbp - (char*)Tail) ;  // Set the length of the new Tail-node
    
    //mergeNodes();
    assert((size_t*)((char*)Tail + Tail->length) <= programBreak);  // Ensure that the new node does not exceed the heap boundary
    ainsert++;
    mergeNodes();
}

void HmmFree(void *ptr) {
    if (ptr == NULL) {
        return;  // Do nothing if the pointer is NULL
    }
    
    if(!isFlistAvailable) return;    
    
    fnode* blockToFree = (fnode*)((char*)ptr - sizeof(fnode));  // Get the free node from the pointer

    // Insert block into the free list
    blockToFree->next = Head;  // Set the next pointer of the block to the current head
    blockToFree->prev = NULL;  // Set the previous pointer of the block to NULL
    assert(Head != NULL);  // Assert that the head is not NULL
    Head->prev = blockToFree;  // Update the previous pointer of the old head
    Head = blockToFree;  // Update the head to the new block
	
    mergeNodes();  // Merge adjacent free nodes
    af++;
}

/* This function handles merging of adjacent free nodes */
void mergeNodes(void) {
    // Check if the list is empty or has only one node
    if (!Head || !(Head->next) || Head->next == Tail) {
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
    mergeCount++;
}

void printFreeList() {
    fnode *current = Head;
    while (current != NULL) {
        printf("Node at %p: length = %zu, prev = %p, next = %p\n",
               (void*)current, current->length, (void*)current->prev, (void*)current->next);
        current = current->next;
    }
}
