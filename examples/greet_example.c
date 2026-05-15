/*
 * SPDX-License-Identifier: MIT
 *
 * Minimal example: link against libhello and call its public API.
 */
#include "hello/hello.h"

#include <stdio.h>

int main(void) {
    printf("libhello %s\n", hello_version());

    char buf[64];
    size_t needed = 0;
    if (hello_format(buf, sizeof buf, "Example", &needed) == HELLO_OK) {
        printf("formatted (%zu bytes): %s\n", needed, buf);
    }

    return hello_greet(stdout, "Reader") == HELLO_OK ? 0 : 1;
}
