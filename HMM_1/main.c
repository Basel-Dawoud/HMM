#include <stdio.h>
#include <stdlib.h>
#include "HMM.h"
#include "LinkedList.h"

// Initialize heap manager
void init_heap_manager() {
    HMM_Init();
}

void test_hmm() {
    printf("Testing Heap Memory Manager...\n");

    // Allocate some memory
    void *ptr1 = HmmAlloc(100);
    void *ptr2 = HmmAlloc(200);
    void *ptr3 = HmmAlloc(300);

    if (ptr1) printf("Allocated 100 bytes at %p\n", ptr1);
    if (ptr2) printf("Allocated 200 bytes at %p\n", ptr2);
    if (ptr3) printf("Allocated 300 bytes at %p\n", ptr3);

    // Free some memory
    HmmFree(ptr2);
    HmmFree(ptr1);

    // Allocate more memory to check reusability
    void *ptr4 = HmmAlloc(50);
    void *ptr5 = HmmAlloc(150);

    if (ptr4) printf("Allocated 50 bytes at %p\n", ptr4);
    if (ptr5) printf("Allocated 150 bytes at %p\n", ptr5);

    // Free all allocated memory
    HmmFree(ptr3);
    HmmFree(ptr4);
    HmmFree(ptr5);

    printf("Heap Memory Manager test completed.\n");
}

void test_linked_list() {
    printf("Testing Linked List...\n");

    // Insert nodes into the linked list
    insertatbegin(10);
    insertatbegin(20);
    insertatbegin(30);
    insertatend(40);
    insertatend(50);
    insertafternode(head->next, 25);

    printf("Linked List after insertions: ");
    printList();

    // Delete nodes from the linked list
    deleteatbegin();
    deleteatend();
    deletenode(25);

    printf("Linked List after deletions: ");
    printList();

    printf("Linked List test completed.\n");
}

int main() {
    init_heap_manager();
    test_hmm();
    test_linked_list();
    return 0;
}
