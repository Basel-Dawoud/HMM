#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "heap.h"

#define NUM_ALLOCS 10000
#define MAX_SIZE 10240
#define MAX_ITERATIONS 1000000

typedef struct AllocationRecord {
    void* ptr;
    size_t size;
} AllocationRecord;

void random_alloc_free_test() {
    srand((unsigned int)time(NULL));

    AllocationRecord records[NUM_ALLOCS];
    int allocatedCount = 0;

    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        int action = rand() % 2; // 0 for allocation, 1 for deallocation

        if (action == 0 && allocatedCount < NUM_ALLOCS) {
            // Allocate memory
            size_t size = (size_t)(rand() % MAX_SIZE) + 1;
            void* ptr = HmmAlloc(size);
            if (ptr != NULL) {
                records[allocatedCount].ptr = ptr;
                records[allocatedCount].size = size;
                allocatedCount++;
                printf("Allocated memory of size %zu at address %p\n", size, ptr);
            } else {
                fprintf(stderr, "Allocation failed for size %zu\n", size);
            }
        } else if (action == 1 && allocatedCount > 0) {
            // Randomly choose an allocated record to free
            int index = rand() % allocatedCount;
            void* ptr = records[index].ptr;
            printf("Freeing memory at address %p\n", ptr);
            HmmFree(ptr);

            // Remove the record and shift the remaining records
            for (int j = index; j < allocatedCount - 1; ++j) {
                records[j] = records[j + 1];
            }
            allocatedCount--;
        }
    }

    // Free remaining allocated memory
    for (int i = 0; i < allocatedCount; ++i) {
        printf("Freeing remaining memory at address %p\n", records[i].ptr);
        HmmFree(records[i].ptr);
    }
}

int main() {
    printf("Starting random allocation and deallocation test...\n");
    random_alloc_free_test();
    printf("Test complete.\n");
    return 0;
}
