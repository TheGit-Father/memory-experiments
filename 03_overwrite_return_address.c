/*
 * 03_overwrite_return_address.c
 *
 * Demonstrates: the stack frame layout from CS:APP Figure 3.25/3.40 --
 * a local buffer overflow corrupting the SAVED RETURN ADDRESS, which
 * changes where the program jumps to on `ret`.
 *
 * This is the mechanism CS:APP 3.10.3 describes: locals, saved
 * registers, and the return address all sit in the same stack frame.
 * Overflowing a local array can overwrite the return address, so when
 * the function executes `ret`, control transfers somewhere the
 * programmer never intended (here: a function that's never called
 * directly from main).
 *
 * NOTE: exact byte offsets needed to hit the return address depend on
 * your compiler, stack layout, and whether stack-protector/ASLR are
 * enabled. This program demonstrates the *concept* (corruption ->
 * crash or hijack) without hardcoding fragile offsets, since real
 * exploit-writing is out of scope here -- the point is to see the
 * crash and understand *why* it happens.
 */
#include <stdio.h>
#include <string.h>

void secret_function(void) {
    printf("secret_function() ran -- this should only happen if "
           "control flow was hijacked!\n");
}

void vulnerable(const char *input) {
    char buf[16];
    printf("  buf is at %p\n", (void *)buf);
    strcpy(buf, input); /* unchecked copy -- can overwrite saved %rbp
                            and the return address pushed by `call` */
    printf("  vulnerable() about to return normally...\n");
}

int main(void) {
    char big_input[200];
    memset(big_input, 'A', sizeof(big_input) - 1);
    big_input[sizeof(big_input) - 1] = '\0';

    printf("Calling vulnerable() with a %zu-byte string into a "
           "16-byte buffer.\n", strlen(big_input));
    printf("Expected: the saved return address gets overwritten with "
           "'A' bytes (0x41), so `ret` jumps to an invalid address "
           "and the program crashes (segfault).\n\n");

    vulnerable(big_input);

    /* We normally never reach here once the overflow is large enough
       to corrupt the return address -- the crash happens inside
       vulnerable()'s `ret` instruction. */
    printf("main() resumed normally (overflow didn't reach the "
           "return address this run).\n");
    return 0;
}
