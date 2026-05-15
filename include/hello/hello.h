/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 hello contributors
 *
 * Public API for the hello library.
 *
 * The library produces greeting strings. It is intentionally small; its purpose
 * is to demonstrate a modern, well-factored C project layout. All functions are
 * thread-safe provided that distinct threads pass distinct output streams /
 * buffers. No global state is used.
 */
#ifndef HELLO_HELLO_H
#define HELLO_HELLO_H

#include <stddef.h>
#include <stdio.h>

#include "hello/export.h"
#include "hello/version.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Error codes returned by the hello API. Zero means success. */
typedef enum hello_status {
    HELLO_OK = 0,
    HELLO_ERR_INVALID_ARG = 1, /**< A required argument was NULL or invalid. */
    HELLO_ERR_IO = 2,          /**< An I/O operation against the stream failed. */
    HELLO_ERR_OVERFLOW = 3     /**< The destination buffer was too small. */
} hello_status;

/**
 * Write a greeting for @p name to @p out.
 *
 * @param out  Destination stream. Must not be NULL.
 * @param name Name to greet. If NULL or empty, "World" is used.
 * @return ::HELLO_OK on success, or an error code on failure.
 *
 * @note On success at least one byte is written; on failure the stream may
 *       have been partially written. Callers that need atomicity should
 *       buffer with ::hello_format first.
 */
HELLO_API hello_status hello_greet(FILE *out, const char *name);

/**
 * Format a greeting for @p name into @p buf.
 *
 * @param buf      Destination buffer. Must not be NULL when @p buf_size > 0.
 * @param buf_size Size of @p buf in bytes (including space for the NUL).
 * @param name     Name to greet. If NULL or empty, "World" is used.
 * @param[out] needed
 *                 If non-NULL, receives the number of bytes that would have
 *                 been written excluding the trailing NUL (as with snprintf).
 * @return ::HELLO_OK on success, ::HELLO_ERR_OVERFLOW if @p buf_size was too
 *         small to hold the full greeting plus NUL (output is still
 *         NUL-terminated and truncated), or another error code.
 */
HELLO_API hello_status hello_format(char *buf, size_t buf_size,
                                    const char *name, size_t *needed);

/** Return the library's semantic version string ("MAJOR.MINOR.PATCH"). */
HELLO_API const char *hello_version(void);

/** Return a human-readable description of a status code. Never NULL. */
HELLO_API const char *hello_status_string(hello_status status);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HELLO_HELLO_H */
