/*
 * SPDX-License-Identifier: MIT
 *
 * libFuzzer entry point for hello_format. The function is small enough to be
 * exhaustively reasoned about, but the harness demonstrates the wiring: any
 * future API that takes attacker-controlled bytes should grow a sibling
 * fuzz_* file in this directory.
 *
 * Build with the `fuzz` preset (Clang + -fsanitize=fuzzer,address). Run:
 *   ./build/fuzz/tests/fuzz/fuzz_format -max_total_time=30
 */
#include "hello/hello.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* libFuzzer mandates this exact symbol name. The forward declaration tames
 * -Wmissing-prototypes; the NOLINT silences clang-tidy's case-style check,
 * which we can't satisfy without breaking libFuzzer. */
/* NOLINTNEXTLINE(readability-identifier-naming) */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

/* NOLINTNEXTLINE(readability-identifier-naming) */
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    /* Treat the input as a NUL-terminated name. Inject a NUL ourselves so the
     * fuzzer can't smuggle un-terminated bytes into the API. */
    char *name = (char *)malloc(size + 1);
    if (name == NULL) {
        return 0;
    }
    if (size > 0) {
        memcpy(name, data, size);
    }
    name[size] = '\0';

    char buf[128];
    size_t needed = 0;
    (void)hello_format(buf, sizeof buf, name, &needed);

    /* Also exercise the length-query path. */
    (void)hello_format(NULL, 0, name, &needed);

    free(name);
    return 0;
}
