#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define constants
#define MAX_HEAP_SIZE 1024 * 1024 // 1 MB for example
#define MAX_ALLOC_SIZE 256 // Maximum allocation size in bytes

// Forward declarations of the HMM functions
void *HmmAlloc(size_t size);
void HmmFree(void *ptr);

// Function prototypes for testing
void test_hmm(size_t num_operations);

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Number of operations to perform
    size_t num_operations = 20;
    
    // Run the test
    test_hmm(num_operations);

    return 0;
}

void test_hmm(size_t num_operations) {
    void *allocations[num_operations];
    size_t sizes[num_operations];
    size_t i;

    // Initialize allocation array
    for (i = 0; i < num_operations; ++i) {
        allocations[i] = NULL;
        sizes[i] = rand() % MAX_ALLOC_SIZE + 1; // Random size between 1 and MAX_ALLOC_SIZE
    }

    printf("Testing Heap Memory Manager...\n");

    // Perform random allocations and deallocations
    for (i = 0; i < num_operations; ++i) {
        // Randomly choose to allocate or free memory
        if (rand() % 2 == 0) {
            // Allocate memory
            allocations[i] = HmmAlloc(sizes[i]);
            printf("Allocated %zu bytes at %p\n", sizes[i], allocations[i]);
        } else {
            // Free memory
            if (allocations[i] != NULL) {
                HmmFree(allocations[i]);
                printf("Freed memory at %p\n", allocations[i]);
                allocations[i] = NULL;
            }
        }
    }

    // Free remaining allocations
    for (i = 0; i < num_operations; ++i) {
        if (allocations[i] != NULL) {
            HmmFree(allocations[i]);
            printf("Freed remaining memory at %p\n", allocations[i]);
        }
    }

    printf("Heap Memory Manager test complete.\n");
}
