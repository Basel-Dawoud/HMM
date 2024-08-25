/*HMM.c (Functions & APIs)*/

#include <stddef.h>
#include <stdio.h>
#include "heap.h"

/* Declaration of the static array representing the virtual heap */
static char vHeap[VHEAP_MAX_SIZE];
static char *heapBase = vHeap;
static char *programBreak = vHeap;

/* Global variables for free list */
fnode *Head = NULL;
fnode *Tail = NULL;

/* Adjusts the simulated program break */
void *sbreak(intptr_t increment) {
    char* oldProgBreak = programBreak;
    char* newProgBreak = programBreak + increment;
    if (newProgBreak > (heapBase + VHEAP_MAX_SIZE) || newProgBreak < heapBase) {
        printf("SIGSEGV: Program break exceeded limits\n");    
        return (void *) -1;
    }
    programBreak = newProgBreak;
    return oldProgBreak;
}

/* Initializes the free list */
void freeListInit(void){
    Head  = (fnode*)sbreak(PAGE); /* at the start of the Heap */
    Head->length = (size_t)sbreak(0) - (size_t)Head - sizeof(fnode);
    Head->prev = NULL; /* */
    Head->next = NULL;
    Tail = Head;
}

/* Adds a new free node after the given node */
void* addfnode(fnode* node){
    /* Compute the address of the new node */
    fnode* newNode = (fnode*)((char*)node + sizeof(fnode) + node->length);

    /* Initialize the new node */
    newNode->next = node->next;
    newNode->prev = node;
    if(node->next){
        node->next->prev = newNode;    
    }
    node->next = newNode;

    if(node == Tail){
        Tail = newNode;
	newNode->next = NULL;
    }

    /* Calculate the length of the new free block */
   if(newNode->next){
        newNode->length = (size_t)((char*)node->next - (char*)newNode - META_DATA_SIZE);
    } else{
        newNode->length = (size_t)((char*)sbreak(0) - (char*)newNode - META_DATA_SIZE);
    }

    //node->next = newNode;
   /* Return the new node */
    return newNode;
}

/* This function needs to handle merging of adjacent free nodes */
void *mergeNodes(void){

return NULL;
}

/* Splits a free node if it is larger than the requested block size */
void split(fnode* node, size_t blockSize){
    if(Head == NULL){ /*No linked list yet*/
        freeListInit();
        return;
    } else if(node == Head && node->next == NULL && Head->next == NULL){ /*There is only one node "The Head-Tail Node"*/
        node->length = blockSize;
        node->next = (fnode*)addfnode(node);
        Tail = node->next;
    } else{
        node->length = blockSize;
        node->next = (fnode*)addfnode(node);
    }
}

/* Finds a free node that fits the requested block size using the first-fit strategy */
void *firstFit(size_t blockSize){	
    for(fnode* curr = Head; curr != NULL ; curr = curr->next){
        if(curr->length > blockSize){
            split(curr,blockSize);
            return curr;
        } else if(curr->length == blockSize){
            return curr;
	} 
        /*else /*if (curr->length < blockSize)*/ 
        /*continue; */
	if(curr->next == NULL){
    	    Tail = curr->prev;
	}
    }
    /*if there is no free node fits with this size*/    
    return NULL;
}


void *HmmAlloc(size_t blockSize){
    if (blockSize == 0) {
        return NULL;
    }

    if (Head == NULL) {
        freeListInit();
    }

    fnode* allocBlock = (fnode*)firstFit(blockSize);
    if(allocBlock == NULL){
        if(sbreak(PAGE) == (void*)-1){
            return NULL;
        }
        fnode* tailNode = (fnode*)addfnode(Tail);
        allocBlock = firstFit(blockSize);
    }
    /*Add meta data*/
    allocBlock->length = blockSize;
    allocBlock = (fnode*)((size_t)allocBlock + META_DATA_SIZE);
    
    /*return the pointer after meta data*/
    return allocBlock;
}

void HmmFree(void *ptr){
    fnode* blockToFree = (fnode*)ptr;
    if(blockToFree == NULL){
        /*Do nothing*/
        return;
    }
   
    if(Head == NULL){
        /*Initialize the free list*/
        freeListInit();
        return;            
    }

    /* Insert the freed block back into the free list */
    blockToFree->next = Head;
    blockToFree->prev = NULL;
    if(Head){
    	Head->prev = blockToFree;
    }
    Head = blockToFree; 


    /* Merge adjacent free blocks */
   // mergeNodes();
}
