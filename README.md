# hello

[![CI](https://github.com/jkindrix/hello/actions/workflows/ci.yml/badge.svg)](https://github.com/jkindrix/hello/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C17](https://img.shields.io/badge/C-17-informational.svg)](https://www.iso.org/standard/74528.html)
[![CMake >= 3.20](https://img.shields.io/badge/CMake-%E2%89%A5%203.20-informational.svg)](CMakeLists.txt)

A small, modern C "Hello, World!" — built as a reference for how to lay out and
operate a production-grade C project. It is intentionally tiny so the
*structure* is the lesson.

## What's inside

- A library, `libhello`, with a clean public C API (`include/hello/hello.h`).
- A CLI, `hello`, that wraps the library.
- Unit tests, smoke tests, examples.
- Modern CMake (target-based, install/export, presets).
- Sanitizer & coverage build presets.
- Continuous integration on GitHub Actions across GCC/Clang and Debug/Release.
- Editor, formatter, linter, and Git configuration.

## Requirements

- A C17-capable compiler (GCC ≥ 9, Clang ≥ 9, MSVC ≥ 2019).
- CMake ≥ 3.20.
- Ninja (recommended) or any other CMake generator.

## Build & run

```sh
cmake --preset debug
cmake --build --preset debug
./build/debug/hello
./build/debug/hello Ada Grace "Margaret Hamilton"
./build/debug/hello --version
```

Other presets:

```sh
cmake --preset release         && cmake --build --preset release
cmake --preset asan            && cmake --build --preset asan
cmake --preset tsan            && cmake --build --preset tsan
cmake --preset coverage        && cmake --build --preset coverage
```

## CLI exit codes

| Code | Meaning                                                                  |
| ---- | ------------------------------------------------------------------------ |
| 0    | Success — greeting written, or `--help` / `--version` printed.           |
| 1    | The library returned a non-OK status for at least one greeting.          |
| 2    | I/O error writing to stdout, or an unknown option on the command line.   |

These are stable and suitable for shell scripting.

## Test

```sh
ctest --preset default
```

The suite covers the library's API (default name, empty name, named greeting,
buffer overflow handling, NULL-argument rejection, status strings) and runs
smoke tests against the CLI (`--help`, `--version`, default and named output).

## Install

```sh
cmake --preset release
cmake --build --preset release
cmake --install build/release --prefix /usr/local
```

This installs:

- `bin/hello`           — the CLI executable
- `lib/libhello.a`      — the static library (or `.so` if `BUILD_SHARED_LIBS=ON`)
- `include/hello/*.h`   — public headers
- `lib/cmake/hello/`    — `find_package(hello)` config files

## Consuming from another CMake project

```cmake
find_package(hello 1.0 REQUIRED)
target_link_libraries(my_app PRIVATE hello::hello)
```

Or as a subproject:

```cmake
add_subdirectory(third_party/hello)
target_link_libraries(my_app PRIVATE hello::hello)
```

## Consuming on Windows / MSVC

The CI matrix builds `hello` with MSVC on `windows-latest`. A few notes for
downstream MSVC consumers:

- **Static linkage (default)** — no extra defines needed. `find_package(hello)`
  + `target_link_libraries(my_app PRIVATE hello::hello)` is sufficient.
- **Shared linkage** — build `hello` with `-DBUILD_SHARED_LIBS=ON`. The
  package config tags consumers with `HELLO_USE_SHARED` automatically, so
  the `HELLO_API` macro resolves to `__declspec(dllimport)` on the consumer
  side. If you are *not* using CMake to consume, define `HELLO_USE_SHARED`
  yourself before including `<hello/hello.h>`.
- **AddressSanitizer** — MSVC supports `/fsanitize=address` since VS 2019
  16.9. Set the `asan` CMake preset (which uses `/fsanitize=address` on
  MSVC) and ensure the matching `clang_rt.asan_dynamic-*.dll` is on `PATH`
  when running the binary.
- **Hardening flags** — `_FORTIFY_SOURCE`, RELRO/BIND_NOW, and `-pie` are
  no-ops or unsupported on Windows; the build skips them automatically.
  MSVC's own mitigations (`/GS`, `/guard:cf`, ASLR via `/DYNAMICBASE`) are
  on by default and unaffected.

## Consuming from non-CMake build systems

A `pkg-config` file is installed alongside the CMake config:

```sh
pkg-config --cflags --libs hello
cc my_app.c $(pkg-config --cflags --libs hello) -o my_app
```

## Project layout

```
hello/
├── CMakeLists.txt           Top-level build script
├── CMakePresets.json        Named build/test configurations
├── cmake/                   Helper modules (warnings, sanitizers, packaging)
├── include/hello/           Public headers (installed)
├── src/                     Library implementation + CLI entry point
├── tests/                   Unit, CLI smoke, fuzz, and downstream-consumer tests
├── examples/                Minimal usage example
├── docs/                    Doxygen configuration
├── scripts/                 Developer scripts (format, lint)
└── .github/workflows/       CI definitions
```

## Public API at a glance

```c
#include <hello/hello.h>

hello_status hello_greet(FILE *out, const char *name);
hello_status hello_format(char *buf, size_t buf_size,
                          const char *name, size_t *needed);
const char  *hello_version(void);
const char  *hello_status_string(hello_status status);
```

All functions are thread-safe (no global state). `NULL` or empty names are
treated as `"World"`. Buffers are always NUL-terminated on return.

## Development

- Format: `scripts/format.sh` (runs `clang-format -i` on all sources).
- Lint:   `scripts/lint.sh`   (runs `clang-tidy` against `compile_commands.json`).
- Hooks:  `scripts/install-hooks.sh` wires `core.hooksPath` to `.githooks/`,
  enabling a `clang-format --dry-run -Werror` check on staged C sources at
  commit time.
- Fuzz:   `cmake --build build/fuzz --target fuzz_format` then
  `./build/fuzz/tests/fuzz/fuzz_format tests/fuzz/corpus -max_total_time=60`.
- CI:     see `.github/workflows/ci.yml` for the build/test matrix, sanitizer
  (ASan+UBSan, TSan), lint, coverage, fuzz, downstream-consumer, and Doxygen
  jobs.

## License

[MIT](LICENSE).
