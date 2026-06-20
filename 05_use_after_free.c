/*
 * 05_use_after_free.c
 *
 * Demonstrates: HEAP USE-AFTER-FREE, a different class of heap bug
 * from program 2's heap overflow. Here the memory access is *within*
 * the originally allocated bounds, but happens AFTER free() has
 * already returned that memory to the allocator.
 *
 * This matters for the "heap" side of the stack-vs-heap model:
 * unlike stack memory (automatically reclaimed when a function
 * returns, CS:APP 3.7.1), heap memory is explicitly managed by the
 * programmer. free() does not erase the data or the pointer -- it
 * just marks the memory as reusable. Writing through a stale
 * ("dangling") pointer afterward is undefined behavior: the memory
 * might still look intact (lucky), or it might already have been
 * reused by another allocation (corruption).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    char *msg = malloc(32);
    if (!msg) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    strcpy(msg, "original heap data");
    printf("Before free: %s\n", msg);

    free(msg);                 /* msg now dangles -- memory returned to allocator */

    printf("Writing through the freed (dangling) pointer...\n");
    strcpy(msg, "use-after-free!");  /* BUG: writing to freed memory */

    printf("After free: %s\n", msg); /* BUG: reading freed memory */

    return 0;
}
