/*
 * 01_stack_overflow_strcpy.c
 *
 * Demonstrates: classic STACK buffer overflow.
 *
 * `buf` is a local array -> lives in the current stack frame
 * (see CS:APP 3.7.4, Local Storage on the Stack).
 * strcpy() does no bounds checking, so copying a string longer
 * than 8 bytes into buf[8] writes past the end of the array and
 * into adjacent stack memory -- possibly corrupting saved
 * registers or the return address (CS:APP 3.10.3).
 */
#include <stdio.h>
#include <string.h>

void vulnerable(const char *input) {
    char buf[8];          /* tiny buffer, on purpose */
    strcpy(buf, input);   /* no bounds check -> overflow if input > 7 chars */
    printf("buf contains: %s\n", buf);
}

int main(void) {
    const char *attack_string = "AAAAAAAAAAAAAAAAAAAAAAAA"; /* 24 bytes, way > 8 */
    printf("Copying %zu bytes into an 8-byte stack buffer...\n",
           strlen(attack_string));
    vulnerable(attack_string);
    printf("If you see this, the overflow didn't crash the program "
           "(undefined behavior -- it might 'work' by luck).\n");
    return 0;
}
