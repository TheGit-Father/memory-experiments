/*
 * 02_heap_overflow_malloc.c
 *
 * Demonstrates: HEAP buffer overflow (contrast with stack overflow in
 * program 1).
 *
 * `data` is allocated with malloc() -> lives on the heap, not the
 * stack. The heap grows independently of function calls and is
 * managed manually (no automatic cleanup when the function returns).
 * Writing past the end of a heap allocation corrupts the heap's
 * internal bookkeeping (chunk metadata) instead of a return address.
 * This is a different memory region with different consequences than
 * the stack overflow in program 1.
 */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int n = 5;
    int *data = malloc(n * sizeof(int)); /* 5 ints, on the heap */
    if (!data) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    printf("Allocated %d ints (%zu bytes) on the heap.\n",
           n, n * sizeof(int));
    printf("Heap address: %p\n", (void *)data);
    fflush(stdout); /* make sure the lines above are flushed BEFORE we
                        corrupt memory -- stdout's own internal buffer
                        is itself a heap allocation, and on this
                        system it happens to sit right after `data`.
                        Without this flush, the overflow below can
                        clobber stdio's buffer before it's printed,
                        and you'll see raw garbage bytes appear in the
                        output -- a vivid (if confusing) example of
                        heap corruption reaching into unrelated data. */

    /* Fill the legitimately allocated region */
    for (int i = 0; i < n; i++) {
        data[i] = i * 10;
    }

    /* Bug: loop condition should be i < n, but writes past the end */
    printf("Writing past the end of the allocation...\n");
    fflush(stdout);
    for (int i = 0; i < n + 10; i++) {
        data[i] = i * 10;   /* indices 5..14 are out of bounds */
    }

    printf("data[0] = %d (still valid)\n", data[0]);
    free(data);
    return 0;
}
