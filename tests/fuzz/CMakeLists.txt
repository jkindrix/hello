# Fuzz targets require Clang with libFuzzer support. The Clang gate lives in
# the top-level CMakeLists.txt, which auto-disables HELLO_BUILD_FUZZERS with
# a warning on non-Clang toolchains; reaching this file means the check has
# already passed.

function(hello_add_fuzz_target name)
    add_executable(${name} ${name}.c)
    target_link_libraries(${name} PRIVATE hello::hello)
    target_compile_options(${name} PRIVATE -fsanitize=fuzzer,address,undefined)
    target_link_options(${name}    PRIVATE -fsanitize=fuzzer,address,undefined)
    hello_apply_warnings(${name})
endfunction()

hello_add_fuzz_target(fuzz_format)
hello_add_fuzz_target(fuzz_greet)
