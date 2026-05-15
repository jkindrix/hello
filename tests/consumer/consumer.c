/*
 * SPDX-License-Identifier: MIT
 *
 * Downstream consumer of an installed libhello. Used by CI to verify the
 * find_package(hello) integration end-to-end.
 */
#include <hello/hello.h>

#include <stdio.h>

int main(void) {
    printf("linked against libhello %s\n", hello_version());
    return hello_greet(stdout, "Consumer") == HELLO_OK ? 0 : 1;
}
