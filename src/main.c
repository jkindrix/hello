/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 hello contributors
 *
 * CLI front-end for libhello.
 *
 * Usage:
 *   hello [NAME]...
 *   hello --version
 *   hello --help
 *
 * If no NAME is supplied, greets "World". Multiple NAMEs are greeted in order.
 */
#include "hello/hello.h"

#include <stdio.h>
#include <string.h>

/* Returns 0 on success, non-zero on I/O failure (mirrors main's exit codes). */
static int print_usage(FILE *stream, const char *prog) {
    int rc = fprintf(stream,
                     "Usage: %s [NAME]...\n"
                     "       %s --version\n"
                     "       %s --help\n"
                     "\n"
                     "Print a friendly greeting. With no NAME, greets \"World\".\n",
                     prog, prog, prog);
    return rc < 0 ? 2 : 0;
}

int main(int argc, char *argv[]) {
    const char *prog = (argc > 0 && argv[0] != NULL) ? argv[0] : "hello";

    /* Recognize options only if they appear before any positional argument. */
    if (argc >= 2) {
        const char *arg = argv[1];
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            return print_usage(stdout, prog);
        }
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-V") == 0) {
            if (printf("hello %s\n", hello_version()) < 0) {
                return 2;
            }
            return 0;
        }
        if (arg[0] == '-' && arg[1] != '\0') {
            fprintf(stderr, "%s: unknown option: %s\n", prog, arg);
            print_usage(stderr, prog);
            return 2;
        }
    }

    if (argc < 2) {
        hello_status st = hello_greet(stdout, NULL);
        if (st != HELLO_OK) {
            fprintf(stderr, "%s: %s\n", prog, hello_status_string(st));
            return 1;
        }
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        hello_status st = hello_greet(stdout, argv[i]);
        if (st != HELLO_OK) {
            fprintf(stderr, "%s: %s\n", prog, hello_status_string(st));
            return 1;
        }
    }
    return 0;
}
