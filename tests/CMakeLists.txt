add_executable(test_hello test_hello.c)
target_link_libraries(test_hello PRIVATE hello::hello)
hello_apply_warnings(test_hello)
hello_apply_sanitizers(test_hello)

add_test(NAME unit_tests COMMAND test_hello)
set_tests_properties(unit_tests PROPERTIES
    FAIL_REGULAR_EXPRESSION "FAIL:"
    TIMEOUT 30
)

# Smoke tests for the CLI. CTest fails the test on non-zero exit OR if the
# PASS_REGULAR_EXPRESSION doesn't match, so exit code 0 is asserted implicitly.
# Regexes accept an optional \r before the trailing \n so they match on
# Windows, where stdout is text-mode and the line ends in \r\n.
add_test(NAME cli_default_greeting COMMAND hello_cli)
set_tests_properties(cli_default_greeting PROPERTIES
    PASS_REGULAR_EXPRESSION "^Hello, World!\r?\n$"
    TIMEOUT 10
)

add_test(NAME cli_named_greeting COMMAND hello_cli Ada)
set_tests_properties(cli_named_greeting PROPERTIES
    PASS_REGULAR_EXPRESSION "^Hello, Ada!\r?\n$"
    TIMEOUT 10
)

add_test(NAME cli_multi_name_greeting COMMAND hello_cli Ada Grace "Margaret Hamilton")
set_tests_properties(cli_multi_name_greeting PROPERTIES
    PASS_REGULAR_EXPRESSION "Hello, Ada!.*Hello, Grace!.*Hello, Margaret Hamilton!"
    TIMEOUT 10
)

add_test(NAME cli_double_dash_terminator COMMAND hello_cli -- --version)
set_tests_properties(cli_double_dash_terminator PROPERTIES
    PASS_REGULAR_EXPRESSION "^Hello, --version!\r?\n$"
    TIMEOUT 10
)

add_test(NAME cli_double_dash_alone COMMAND hello_cli --)
set_tests_properties(cli_double_dash_alone PROPERTIES
    PASS_REGULAR_EXPRESSION "^Hello, World!\r?\n$"
    TIMEOUT 10
)

add_test(NAME cli_version COMMAND hello_cli --version)
set_tests_properties(cli_version PROPERTIES
    PASS_REGULAR_EXPRESSION "^hello [0-9]+\\.[0-9]+\\.[0-9]+\r?\n$"
    TIMEOUT 10
)

add_test(NAME cli_help COMMAND hello_cli --help)
set_tests_properties(cli_help PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage:"
    TIMEOUT 10
)

# Unknown-option path: must exit 2 AND print "unknown option" to stderr.
# WILL_FAIL+PASS_REGULAR_EXPRESSION don't compose the way you'd expect
# (regex match marks the test passed before WILL_FAIL inverts it), so wrap
# in bash and assert both conditions explicitly.
if(CMAKE_HOST_UNIX)
    add_test(NAME cli_unknown_option
        COMMAND bash -c "out=$($<TARGET_FILE:hello_cli> --bogus 2>&1); rc=$?; echo \"$out\"; test $rc -eq 2 && echo \"$out\" | grep -q 'unknown option'"
    )
    set_tests_properties(cli_unknown_option PROPERTIES TIMEOUT 10)
endif()

# Broken-pipe path: HELLO_ERR_IO from hello_greet must surface as CLI exit
# code 2 (the I/O-error code), not 1. The shell wrapper closes the pipe's
# read end so the CLI's first write fails. POSIX-only; the SIGPIPE-ignore
# is required because the kernel would otherwise kill the writer before
# fprintf returns.
if(CMAKE_HOST_UNIX)
    add_test(NAME cli_broken_pipe_exit_code
        COMMAND bash -c "trap '' PIPE; stdbuf -o0 $<TARGET_FILE:hello_cli> Ada | head -n 0; test \${PIPESTATUS[0]} -eq 2"
    )
    set_tests_properties(cli_broken_pipe_exit_code PROPERTIES TIMEOUT 10)
endif()

# Visibility plumbing check: only meaningful for shared builds on ELF, where
# a leaked private symbol would actually appear in the dynamic symbol table.
# Static builds expose everything to linkers regardless, so the test is a
# no-op there.
if(BUILD_SHARED_LIBS AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_test(NAME symbol_export_check
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/check_exports.sh"
            $<TARGET_FILE:hello>
    )
    set_tests_properties(symbol_export_check PROPERTIES TIMEOUT 10)
endif()

# Policy test: every HELLO_API declaration in the public header must carry a
# Doxygen comment. Same shape as symbol_export_check: the contract lives in
# the README's "Versioning & ABI stability" section, and this test makes a
# future drift visible immediately rather than at the next Doxygen build.
if(CMAKE_HOST_UNIX)
    add_test(NAME doxygen_coverage_check
        COMMAND bash
            "${CMAKE_CURRENT_SOURCE_DIR}/check_doxygen_coverage.sh"
            "${CMAKE_SOURCE_DIR}/include/hello/hello.h"
    )
    set_tests_properties(doxygen_coverage_check PROPERTIES TIMEOUT 10)
endif()

# Bake sanitizer runtime options into every test so local `ctest` and the CI
# job see identical strictness. Previously these lived only in the CI env
# blocks; two developers running the same preset could otherwise get
# different leak-detection / halt-on-error behavior.
get_property(_hello_all_tests DIRECTORY PROPERTY TESTS)
if(HELLO_ENABLE_ASAN OR HELLO_ENABLE_UBSAN)
    set(_asan_env "ASAN_OPTIONS=detect_leaks=1:strict_string_checks=1:halt_on_error=1")
    set(_ubsan_env "UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1")
    foreach(_t IN LISTS _hello_all_tests)
        set_property(TEST ${_t} APPEND PROPERTY
            ENVIRONMENT "${_asan_env}" "${_ubsan_env}")
    endforeach()
endif()
if(HELLO_ENABLE_TSAN)
    foreach(_t IN LISTS _hello_all_tests)
        set_property(TEST ${_t} APPEND PROPERTY
            ENVIRONMENT "TSAN_OPTIONS=halt_on_error=1:second_deadlock_stack=1")
    endforeach()
endif()
if(HELLO_ENABLE_MSAN)
    foreach(_t IN LISTS _hello_all_tests)
        set_property(TEST ${_t} APPEND PROPERTY
            ENVIRONMENT "MSAN_OPTIONS=halt_on_error=1:print_stats=0")
    endforeach()
endif()
