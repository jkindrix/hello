# fuzz_format seed corpus

Each file here is one input to `LLVMFuzzerTestOneInput`. The fuzzer treats the
bytes as the `name` argument to `hello_format` (NUL-terminated by the harness).
Seeds cover the obvious edge classes — empty input, ASCII names, spaces,
embedded NULs, long inputs that exceed the formatter's stack buffer, and
multi-byte UTF-8 — so the fuzzer starts in a useful neighborhood rather than
mutating from scratch.

Run locally:

```sh
cmake -S . -B build/fuzz -DHELLO_BUILD_FUZZERS=ON -DCMAKE_C_COMPILER=clang
cmake --build build/fuzz --target fuzz_format
./build/fuzz/tests/fuzz/fuzz_format tests/fuzz/corpus -max_total_time=60
```

To merge new finds back into the seed set:

```sh
./build/fuzz/tests/fuzz/fuzz_format -merge=1 tests/fuzz/corpus path/to/new/inputs
```
