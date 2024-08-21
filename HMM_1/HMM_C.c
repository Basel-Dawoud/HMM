#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define VHEAP_MAX_SIZE 1024 * 1024 // Define heap size as 1MB for demonstration

// Virtual heap array
static char vHeap[VHEAP_MAX_SIZE];

// Global variables for heap management
void *heapBase = vHeap;
void *programBreak = vHeap;
void *currNodePtr = vHeap;
int available = VHEAP_MAX_SIZE;

// Free list node structure
struct fnode {
    size_t size;
    struct fnode *next;
};

// Free list head
struct fnode *freeListHead = NULL;

// Function prototypes
void *sbreak(void);
void *HmmAlloc(size_t size);
void HmmFree(void *ptr);
void updateFreeList(struct fnode *node);
void freeListInit(void);
void *firstFit(size_t size);
void printFreeList(void);

// Initialize the free list
void freeListInit(void) {
    freeListHead = (struct fnode *)heapBase;
    freeListHead->size = VHEAP_MAX_SIZE;
    freeListHead->next = NULL;
}

// Simulate the break system call
void *sbreak(void) {
    return programBreak;
}

// Allocate memory from the simulated heap
void *HmmAlloc(size_t size) {
    if (size == 0 || size > available) return NULL;

    // Align size to multiple of sizeof(size_t)
    size = (size + sizeof(size_t) - 1) & ~(sizeof(size_t) - 1);

    // Find a suitable block
    struct fnode *prev = NULL;
    struct fnode *curr = freeListHead;
    struct fnode *bestFit = NULL;
    while (curr) {
        if (curr->size >= size) {
            bestFit = curr;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (!bestFit) return NULL;

    // Split the block if necessary
    if (bestFit->size > size + sizeof(struct fnode)) {
        struct fnode *newNode = (struct fnode *)((char *)bestFit + sizeof(struct fnode) + size);
        newNode->size = bestFit->size - size - sizeof(struct fnode);
        newNode->next = bestFit->next;
        bestFit->size = size;
        bestFit->next = newNode;
    } else {
        if (prev) {
            prev->next = bestFit->next;
        } else {
            freeListHead = bestFit->next;
        }
    }

    available -= bestFit->size;
    return (char *)bestFit + sizeof(struct fnode);
}

// Free memory and add it to the free list
void HmmFree(void *ptr) {
    if (!ptr) return;

    // Get the block header
    struct fnode *node = (struct fnode *)((char *)ptr - sizeof(struct fnode));
    available += node->size;

    // Insert the block into the free list
    updateFreeList(node);
}

// Update the free list with the freed block
void updateFreeList(struct fnode *node) {
    struct fnode *prev = NULL;
    struct fnode *curr = freeListHead;

    // Find the correct position to insert
    while (curr && (char *)curr < (char *)node) {
        prev = curr;
        curr = curr->next;
    }

    // Merge with adjacent blocks if possible
    if (prev && (char *)prev + prev->size + sizeof(struct fnode) == (char *)node) {
        prev->size += node->size + sizeof(struct fnode);
        node = prev;
    } else {
        node->next = curr;
        if (prev) {
            prev->next = node;
        } else {
            freeListHead = node;
        }
    }

    if (curr && (char *)node + node->size + sizeof(struct fnode) == (char *)curr) {
        node->size += curr->size + sizeof(struct fnode);
        node->next = curr->next;
    }
}

// Print the free list (for debugging)
void printFreeList() {
    struct fnode *curr = freeListHead;
    printf("Free List:\n");
    while (curr) {
        printf("Block of size %zu at %p\n", curr->size, curr);
        curr = curr->next;
    }
}

// Linked List operations

struct node {
    int data;
    struct node *next;
};

struct node *head = NULL;

// Insert at the beginning of the linked list
void insertatbegin(int data) {
    struct node *newNode = (struct node *)HmmAlloc(sizeof(struct node));
    if (!newNode) return;

    newNode->data = data;
    newNode->next = head;
    head = newNode;
}

// Insert at the end of the linked list
void insertatend(int data) {
    struct node *newNode = (struct node *)HmmAlloc(sizeof(struct node));
    if (!newNode) return;

    newNode->data = data;
    newNode->next = NULL;

    if (!head) {
        head = newNode;
    } else {
        struct node *curr = head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = newNode;
    }
}

// Insert after a specific node in the linked list
void insertafternode(struct node *list, int data) {
    struct node *newNode = (struct node *)HmmAlloc(sizeof(struct node));
    if (!newNode) return;

    newNode->data = data;
    newNode->next = list->next;
    list->next = newNode;
}

// Delete the first node in the linked list
void deleteatbegin() {
    if (head) {
        struct node *temp = head;
        head = head->next;
        HmmFree(temp);
    }
}

// Delete the last node in the linked list
void deleteatend() {
    if (!head) return;

    if (!head->next) {
        HmmFree(head);
        head = NULL;
    } else {
        struct node *prev = NULL;
        struct node *curr = head;
        while (curr->next) {
            prev = curr;
            curr = curr->next;
        }
        prev->next = NULL;
        HmmFree(curr);
    }
}

// Delete a node with a specific value in the linked list
void deletenode(int key) {
    struct node *curr = head;
    struct node *prev = NULL;

    while (curr && curr->data != key) {
        prev = curr;
        curr = curr->next;
    }

    if (curr) {
        if (prev) {
            prev->next = curr->next;
        } else {
            head = curr->next;
        }
        HmmFree(curr);
    }
}

// Print the linked list
void printList() {
    struct node *p = head;
    printf("[");
    while (p) {
        printf(" %d ", p->data);
        p = p->next;
    }
    printf("]\n");
}

int main(void) {
    // Initialize the heap manager
    freeListInit();

    // Test heap memory manager
    printf("Testing Heap Memory Manager...\n");

    void *ptr1 = HmmAlloc(100);
    void *ptr2 = HmmAlloc(200);
    void *ptr3 = HmmAlloc(300);

    if (ptr1) printf("Allocated 100 bytes at %p\n", ptr1);
    if (ptr2) printf("Allocated 200 bytes at %p\n", ptr2);
    if (ptr3) printf("Allocated 300 bytes at %p\n", ptr3);

    HmmFree(ptr2);
    HmmFree(ptr1);

    void *ptr4 = HmmAlloc(50);
    void *ptr5 = HmmAlloc(150);

    if (ptr4) printf("Allocated 50 bytes at %p\n", ptr4);
    if (ptr5) printf("Allocated 150 bytes at %p\n", ptr5);

    HmmFree(ptr3);
    HmmFree(ptr4);
    HmmFree(ptr5);

    printFreeList();

    // Test linked list operations
    printf("Testing Linked List...\n");

    insertatbegin(10);
    insertatbegin(20);
    insertatbegin(30);
    insertatend(40);
    insertatend(50);
    insertafternode(head->next, 25);

    printf("Linked List after insertions: ");
    printList();

    deleteatbegin();
    deleteatend();
    deletenode(25);

    printf("Linked List after deletions: ");
    printList();

    return 0;
}
