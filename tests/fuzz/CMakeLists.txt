# Fuzz targets require Clang with libFuzzer support. The HELLO_BUILD_FUZZERS
# option must be set explicitly; we don't auto-detect because the resulting
# binaries are not usable as ordinary tests.

if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "HELLO_BUILD_FUZZERS=ON requires Clang (got ${CMAKE_C_COMPILER_ID})")
endif()

add_executable(fuzz_format fuzz_format.c)
target_link_libraries(fuzz_format PRIVATE hello::hello)
target_compile_options(fuzz_format PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_format    PRIVATE -fsanitize=fuzzer,address,undefined)
hello_apply_warnings(fuzz_format)
