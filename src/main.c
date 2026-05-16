/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 hello contributors
 */
/**
 * @file main.c
 * @brief CLI front-end for libhello.
 *
 * Usage:
 *   hello [NAME]...
 *   hello -- [NAME]...
 *   hello --version
 *   hello --help
 *
 * If no NAME is supplied, greets "World". Multiple NAMEs are greeted in order.
 * A bare "--" ends option processing so names beginning with '-' can be used.
 *
 * Exit codes:
 *   - 0  success (greeting written, or --help / --version printed)
 *   - 1  the library returned a non-I/O error status (e.g. invalid argument)
 *   - 2  an I/O error occurred writing to stdout (whether detected directly
 *        in this front-end or surfaced as HELLO_ERR_IO from the library),
 *        or an unknown option was passed on the command line
 */
#include "hello/hello.h"

#include <stdio.h>
#include <string.h>

/* Issue one greeting and translate the library status into an exit code.
 *   HELLO_OK     -> 0
 *   HELLO_ERR_IO -> 2 (I/O failure surfaces as the I/O exit code)
 *   other        -> 1 (logic errors)
 * Errors are reported to stderr before returning. */
static int greet_or_exit(const char *prog, const char *name) {
    hello_status st = hello_greet(stdout, name);
    if (st == HELLO_OK) {
        return 0;
    }
    fprintf(stderr, "%s: %s\n", prog, hello_status_string(st));
    return st == HELLO_ERR_IO ? 2 : 1;
}

/* Returns 0 on success, non-zero on I/O failure (mirrors main's exit codes). */
static int print_usage(FILE *stream, const char *prog) {
    int rc = fprintf(stream,
                     "Usage: %s [NAME]...\n"
                     "       %s -- [NAME]...\n"
                     "       %s {--version | -V}\n"
                     "       %s {--help | -h}\n"
                     "\n"
                     "Print a friendly greeting. With no NAME, greets \"World\".\n"
                     "Use \"--\" to greet names that begin with \"-\".\n",
                     prog, prog, prog, prog);
    return rc < 0 ? 2 : 0;
}

int main(int argc, char *argv[]) {
    const char *prog = (argc > 0 && argv[0] != NULL) ? argv[0] : "hello";

    /* Recognize options only if they appear before any positional argument.
     * A bare "--" terminates option processing; subsequent arguments are
     * treated as names even if they begin with '-'. */
    int first_name = 1;
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
        if (strcmp(arg, "--") == 0) {
            first_name = 2;
        } else if (arg[0] == '-' && arg[1] != '\0') {
            fprintf(stderr, "%s: unknown option: %s\n", prog, arg);
            print_usage(stderr, prog);
            return 2;
        }
    }

    if (first_name >= argc) {
        return greet_or_exit(prog, NULL);
    }

    for (int i = first_name; i < argc; ++i) {
        int rc = greet_or_exit(prog, argv[i]);
        if (rc != 0) {
            return rc;
        }
    }
    return 0;
}
