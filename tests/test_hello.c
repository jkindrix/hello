/*
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for libhello. Uses a small in-file harness so the project has no
 * mandatory external test dependency. Each TEST() runs in isolation; any
 * failed assertion prints a diagnostic and increments the failure counter.
 */
/* Enable POSIX prototypes (fdopen, pipe) under strict C17. Must precede any
 * system header includes. */
#if defined(__unix__) || defined(__APPLE__)
#  ifndef _POSIX_C_SOURCE
/* _POSIX_C_SOURCE is the standard feature-test macro defined by POSIX
 * itself; the underscore prefix is mandatory, not a style violation. */
/* NOLINTNEXTLINE(readability-identifier-naming) */
#    define _POSIX_C_SOURCE 200809L
#  endif
#endif

#include "hello/hello.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__unix__) || defined(__APPLE__)
#  define HELLO_HAVE_PIPE_IO_TEST 1
#  include <signal.h>
#  include <unistd.h>
#endif

static int g_failures = 0;
static int g_tests = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            ++g_failures;                                                      \
        }                                                                      \
    } while (0)

#define CHECK_STR_EQ(a, b)                                                     \
    do {                                                                       \
        const char *_a = (a);                                                  \
        const char *_b = (b);                                                  \
        if (_a == NULL || _b == NULL || strcmp(_a, _b) != 0) {                 \
            fprintf(stderr,                                                    \
                    "  FAIL: %s:%d: expected \"%s\", got \"%s\"\n", __FILE__,  \
                    __LINE__, _b ? _b : "(null)", _a ? _a : "(null)");         \
            ++g_failures;                                                      \
        }                                                                      \
    } while (0)

#define TEST(name)                                                             \
    static void name(void);                                                    \
    static void run_##name(void) {                                             \
        ++g_tests;                                                             \
        fprintf(stderr, "[ RUN  ] %s\n", #name);                               \
        int before = g_failures;                                               \
        name();                                                                \
        fprintf(stderr, "[ %s ] %s\n",                                         \
                g_failures == before ? " OK " : "FAIL", #name);                \
    }                                                                          \
    static void name(void)

TEST(format_default_name_is_world) {
    char buf[64];
    size_t needed = 0;
    hello_status st = hello_format(buf, sizeof buf, NULL, &needed);
    CHECK(st == HELLO_OK);
    CHECK_STR_EQ(buf, "Hello, World!");
    CHECK(needed == strlen("Hello, World!"));
}

TEST(format_empty_name_is_world) {
    char buf[64];
    hello_status st = hello_format(buf, sizeof buf, "", NULL);
    CHECK(st == HELLO_OK);
    CHECK_STR_EQ(buf, "Hello, World!");
}

TEST(format_named) {
    char buf[64];
    hello_status st = hello_format(buf, sizeof buf, "Ada", NULL);
    CHECK(st == HELLO_OK);
    CHECK_STR_EQ(buf, "Hello, Ada!");
}

TEST(format_overflow_truncates_and_reports) {
    char buf[8]; /* "Hello, World!" needs 14 bytes incl. NUL. */
    size_t needed = 0;
    hello_status st = hello_format(buf, sizeof buf, "World", &needed);
    CHECK(st == HELLO_ERR_OVERFLOW);
    CHECK(needed == strlen("Hello, World!"));
    /* Must remain NUL-terminated. */
    CHECK(memchr(buf, '\0', sizeof buf) != NULL);
}

TEST(format_rejects_null_buf_with_nonzero_size) {
    hello_status st = hello_format(NULL, 16, "x", NULL);
    CHECK(st == HELLO_ERR_INVALID_ARG);
}

TEST(format_allows_zero_size_query) {
    size_t needed = 0;
    hello_status st = hello_format(NULL, 0, "Ada", &needed);
    /* With buf_size == 0 the call is a length query; should report overflow
     * but still set `needed` so callers can size their buffer. */
    CHECK(st == HELLO_ERR_OVERFLOW);
    CHECK(needed == strlen("Hello, Ada!"));
}

TEST(greet_rejects_null_stream) {
    hello_status st = hello_greet(NULL, "x");
    CHECK(st == HELLO_ERR_INVALID_ARG);
}

TEST(greet_reports_io_error_on_read_only_stream) {
    /* Portable HELLO_ERR_IO coverage: open the null device for reading and
     * write to it. fprintf goes through the FILE's write handler which
     * eventually invokes the OS write() against an O_RDONLY descriptor,
     * which fails with EBADF on POSIX and the Windows equivalent on MSVC.
     * Unbuffered mode so the failure is visible at fprintf's return rather
     * than deferred to fflush. */
    const char *null_dev =
#ifdef _WIN32
        "NUL";
#else
        "/dev/null";
#endif
    FILE *fp = fopen(null_dev, "rb");
    if (fp == NULL) {
        fprintf(stderr, "  SKIP: cannot open %s for reading\n", null_dev);
        return;
    }
    setvbuf(fp, NULL, _IONBF, 0);
    hello_status st = hello_greet(fp, "Ada");
    CHECK(st == HELLO_ERR_IO);
    fclose(fp);
}

TEST(greet_writes_expected_bytes) {
    /* tmpfile() gives us a binary-safe temp stream we can rewind and read. */
    FILE *fp = tmpfile();
    if (fp == NULL) {
        fprintf(stderr, "  SKIP: tmpfile() unavailable\n");
        return;
    }
    hello_status st = hello_greet(fp, "Grace");
    CHECK(st == HELLO_OK);
    rewind(fp);
    char buf[64] = {0};
    size_t n = fread(buf, 1, sizeof buf - 1, fp);
    buf[n] = '\0';
    CHECK_STR_EQ(buf, "Hello, Grace!\n");
    fclose(fp);
}

#ifdef HELLO_HAVE_PIPE_IO_TEST
TEST(greet_reports_io_error_on_broken_pipe) {
    /* Force fprintf to fail by writing to the write-end of a pipe whose
     * read-end has been closed. SIGPIPE must be ignored or the process
     * dies before fprintf returns. Unbuffered mode ensures the write
     * reaches the kernel immediately. */
    void (*old_handler)(int) = signal(SIGPIPE, SIG_IGN);
    int fds[2];
    if (pipe(fds) != 0) {
        fprintf(stderr, "  SKIP: pipe() unavailable\n");
        signal(SIGPIPE, old_handler);
        return;
    }
    close(fds[0]);
    FILE *fp = fdopen(fds[1], "w");
    if (fp == NULL) {
        close(fds[1]);
        signal(SIGPIPE, old_handler);
        fprintf(stderr, "  SKIP: fdopen() unavailable\n");
        return;
    }
    setvbuf(fp, NULL, _IONBF, 0);
    hello_status st = hello_greet(fp, "Ada");
    CHECK(st == HELLO_ERR_IO);
    fclose(fp);
    signal(SIGPIPE, old_handler);
}
#endif

TEST(version_is_nonempty) {
    const char *v = hello_version();
    CHECK(v != NULL);
    if (v == NULL) {
        return; /* Avoid dereferencing if the contract is already broken. */
    }
    CHECK(v[0] != '\0');
}

TEST(status_string_covers_all_codes) {
    CHECK_STR_EQ(hello_status_string(HELLO_OK), "ok");
    CHECK(hello_status_string(HELLO_ERR_INVALID_ARG)[0] != '\0');
    CHECK(hello_status_string(HELLO_ERR_IO)[0] != '\0');
    CHECK(hello_status_string(HELLO_ERR_OVERFLOW)[0] != '\0');
    /* Unknown codes still produce a non-empty string. */
    CHECK(hello_status_string((hello_status)999)[0] != '\0');
}

int main(void) {
    run_format_default_name_is_world();
    run_format_empty_name_is_world();
    run_format_named();
    run_format_overflow_truncates_and_reports();
    run_format_rejects_null_buf_with_nonzero_size();
    run_format_allows_zero_size_query();
    run_greet_rejects_null_stream();
    run_greet_reports_io_error_on_read_only_stream();
    run_greet_writes_expected_bytes();
#ifdef HELLO_HAVE_PIPE_IO_TEST
    run_greet_reports_io_error_on_broken_pipe();
#endif
    run_version_is_nonempty();
    run_status_string_covers_all_codes();

    fprintf(stderr, "\n%d test(s), %d failure(s)\n", g_tests, g_failures);
    return g_failures == 0 ? 0 : 1;
}
