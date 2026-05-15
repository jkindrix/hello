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
