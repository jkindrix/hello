/*
 * SPDX-License-Identifier: MIT
 *
 * libFuzzer harness for the streaming hello_greet path. The format path is
 * covered by fuzz_format.c; this harness exercises the FILE *-writing
 * counterpart so the I/O code (and any future buffering / locking machinery
 * around it) gets equivalent coverage.
 *
 * The output stream is opened once in LLVMFuzzerInitialize and reused, so
 * we measure mutator throughput rather than fopen/fclose overhead.
 */
#include "hello/hello.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *g_sink = NULL;

/* libFuzzer's signature is fixed; we don't control const-ness of argc/argv. */
/* NOLINTNEXTLINE(readability-identifier-naming,readability-non-const-parameter) */
int LLVMFuzzerInitialize(int *argc, char ***argv);
/* NOLINTNEXTLINE(readability-identifier-naming) */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/* NOLINTNEXTLINE(readability-identifier-naming,readability-non-const-parameter) */
int LLVMFuzzerInitialize(int *argc, char ***argv) {
    (void)argc;
    (void)argv;
    /* Sink writes to the null device so we exercise the streaming path
     * without bloating disk usage. NUL is the Windows equivalent if the
     * fuzzer ever runs there. */
    const char *null_dev =
#ifdef _WIN32
        "NUL";
#else
        "/dev/null";
#endif
    g_sink = fopen(null_dev, "wb");
    if (g_sink == NULL) {
        return 1;
    }
    setvbuf(g_sink, NULL, _IONBF, 0);
    return 0;
}

/* NOLINTNEXTLINE(readability-identifier-naming) */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (g_sink == NULL) {
        return 0;
    }
    char *name = (char *)malloc(size + 1);
    if (name == NULL) {
        return 0;
    }
    if (size > 0) {
        memcpy(name, data, size);
    }
    name[size] = '\0';

    (void)hello_greet(g_sink, name);

    free(name);
    return 0;
}
