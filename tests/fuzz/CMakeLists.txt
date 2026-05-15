# Fuzz targets require Clang with libFuzzer support. The Clang gate lives in
# the top-level CMakeLists.txt, which auto-disables HELLO_BUILD_FUZZERS with
# a warning on non-Clang toolchains; reaching this file means the check has
# already passed.

add_executable(fuzz_format fuzz_format.c)
target_link_libraries(fuzz_format PRIVATE hello::hello)
target_compile_options(fuzz_format PRIVATE -fsanitize=fuzzer,address,undefined)
target_link_options(fuzz_format    PRIVATE -fsanitize=fuzzer,address,undefined)
hello_apply_warnings(fuzz_format)
