/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 hello contributors
 */
/**
 * @file hello.c
 * @brief Implementation of the hello library.
 *
 * Holds the formatting and streaming routines declared in
 * `include/hello/hello.h`. No global state; the only shared resource is the
 * caller-supplied `FILE *` in ::hello_greet.
 */
#include "hello/hello.h"

#include <stdio.h>

/* Internal: choose the effective name, substituting "World" for NULL/empty. */
static const char *hello_effective_name(const char *name) {
    if (name == NULL || name[0] == '\0') {
        return "World";
    }
    return name;
}

const char *hello_version(void) {
    return HELLO_VERSION_STRING;
}

const char *hello_status_string(hello_status status) {
    switch (status) {
    case HELLO_OK:
        return "ok";
    case HELLO_ERR_INVALID_ARG:
        return "invalid argument";
    case HELLO_ERR_IO:
        return "I/O error";
    case HELLO_ERR_OVERFLOW:
        return "output buffer too small";
    default:
        return "unknown error";
    }
}

hello_status hello_format(char *buf, size_t buf_size, const char *name, size_t *needed) {
    if (buf_size > 0 && buf == NULL) {
        return HELLO_ERR_INVALID_ARG;
    }

    const char *who = hello_effective_name(name);

    /* snprintf returns the number of bytes that *would* be written excluding
     * the trailing NUL, or a negative value on encoding error. */
    int written = snprintf(buf, buf_size, "Hello, %s!", who);
    if (written < 0) {
        return HELLO_ERR_IO;
    }

    if (needed != NULL) {
        *needed = (size_t)written;
    }

    if ((size_t)written >= buf_size) {
        /* snprintf NUL-terminates whatever fit (when buf_size > 0). */
        return HELLO_ERR_OVERFLOW;
    }
    return HELLO_OK;
}

hello_status hello_greet(FILE *out, const char *name) {
    if (out == NULL) {
        return HELLO_ERR_INVALID_ARG;
    }

    const char *who = hello_effective_name(name);

    /* fprintf returns a negative value on output error. */
    if (fprintf(out, "Hello, %s!\n", who) < 0) {
        return HELLO_ERR_IO;
    }
    return HELLO_OK;
}
