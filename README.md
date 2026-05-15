# hello

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

## Project layout

```
hello/
├── CMakeLists.txt           Top-level build script
├── CMakePresets.json        Named build/test configurations
├── cmake/                   Helper modules (warnings, sanitizers, packaging)
├── include/hello/           Public headers (installed)
├── src/                     Library implementation + CLI entry point
├── tests/                   Unit and CLI smoke tests
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
- CI:     see `.github/workflows/ci.yml` for the build/test matrix, sanitizer,
  lint, coverage, and Doxygen jobs.

## License

[MIT](LICENSE).
