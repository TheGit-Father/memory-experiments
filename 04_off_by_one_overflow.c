/*
 * 04_off_by_one_overflow.c
 *
 * Demonstrates: a subtle OFF-BY-ONE overflow -- the kind that's easy
 * to miss in review. Unlike programs 1 and 3 (a huge, obvious
 * overflow), this one writes exactly ONE byte past the end of the
 * array. It's small enough that plain execution often looks "fine,"
 * but it is still undefined behavior and corrupts whatever memory
 * sits right after the array on the stack (often padding, a saved
 * register byte, or another local variable -- see CS:APP 3.9.3 on
 * data alignment for why there's often padding there to silently
 * absorb small overflows, which is exactly why these bugs hide so
 * well without a tool like ASan).
 */
#include <stdio.h>

int main(void) {
    char name[5];     /* room for exactly 5 characters, no null terminator slot */
    char marker[5] = "SAFE"; /* a neighboring stack variable to show corruption */

    printf("Before overflow: marker = \"%s\"\n", marker);

    /* Bug: loop runs i = 0..5 inclusive -> 6 writes into a 5-byte array */
    for (int i = 0; i <= 5; i++) {
        name[i] = 'X';   /* name[5] is one byte out of bounds */
    }

    printf("After writing 6 bytes into a 5-byte array...\n");
    printf("marker = \"%s\" (may be corrupted if name[5] landed in marker)\n",
           marker);

    return 0;
}
