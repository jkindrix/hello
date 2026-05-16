# hello

[![CI](https://github.com/jkindrix/hello/actions/workflows/ci.yml/badge.svg)](https://github.com/jkindrix/hello/actions/workflows/ci.yml)
[![CodeQL](https://github.com/jkindrix/hello/actions/workflows/codeql.yml/badge.svg)](https://github.com/jkindrix/hello/actions/workflows/codeql.yml)
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

Example output:

```text
$ ./build/debug/hello
Hello, World!

$ ./build/debug/hello Ada Grace "Margaret Hamilton"
Hello, Ada!
Hello, Grace!
Hello, Margaret Hamilton!

$ ./build/debug/hello --version
hello 1.0.0
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
buffer overflow handling, NULL-argument rejection, I/O error on broken pipe,
status strings) and runs smoke tests against the CLI (`--help`, `--version`,
`--` end-of-options terminator, default, named, and multi-name output).

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

API reference (Doxygen) is published from `main` to
<https://jkindrix.github.io/hello/>.

## Versioning & ABI stability

`hello` follows [Semantic Versioning](https://semver.org/):

- **Major** (`X.0.0`) — may break the public C API or ABI. Source and binary
  consumers may need to be updated and rebuilt.
- **Minor** (`1.X.0`) — adds API surface without breaking existing callers.
  Shared-library `SOVERSION` is preserved; old binaries keep working.
- **Patch** (`1.0.X`) — bug fixes and documentation only. No API or ABI change.

The public surface is exactly the symbols declared in `include/hello/hello.h`
and tagged with `HELLO_API`. Everything else — including any symbol with a
name beginning with `hello_` that is *not* declared in that header — is
private and may change without notice. The `shared-build` CI job runs a
`symbol_export_check` test on Linux that enforces this on every push.

`SOVERSION` is set to the major version, so `libhello.so.1` is the binary
contract for the 1.x series.

## Troubleshooting

Common consumer failure modes and their fixes:

| Symptom | Cause | Fix |
| --- | --- | --- |
| `find_package(hello)` reports "not found". | The install prefix isn't on CMake's search path. | Pass `-DCMAKE_PREFIX_PATH=<your install prefix>` to your project's `cmake` configure, or set `hello_DIR` to the directory containing `helloConfig.cmake`. |
| MSVC shared link fails with unresolved `hello_*` externs. | Consumer compiled without the `dllimport` annotation. | Define `HELLO_USE_SHARED` before `#include <hello/hello.h>`. CMake's `find_package(hello)` does this automatically; non-CMake consumers must do it themselves. |
| `pkg-config --cflags --libs hello` prints "not found". | Install prefix's pkgconfig dir isn't on `PKG_CONFIG_PATH`. | `export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig:$PKG_CONFIG_PATH`. The `.pc` file itself is relocatable, so it doesn't matter where you put the install. |
| Sanitizer build aborts with `unexpected memory mapping`. | Linux kernel ≥ 6.x with Clang ≤ 15: ASLR entropy exceeds the sanitizer shadow region. | Use Clang ≥ 16 (`CC=clang-19 cmake --preset asan`), or lower the entropy: `sudo sysctl -w vm.mmap_rnd_bits=28`. Detailed in [CONTRIBUTING.md](CONTRIBUTING.md). |
| Fuzz target fails to link with `libclang_rt.fuzzer-*.a: No such file`. | Debian splits Clang's sanitizer/fuzzer runtimes into a separate package. | `sudo apt install -y libclang-rt-19-dev` (or the version matching your `clang`). |

## Development

- Format: `scripts/format.sh` (runs `clang-format -i` on all sources).
- Lint:   `scripts/lint.sh`   (runs `clang-tidy` against `compile_commands.json`).
- Hooks:  `scripts/install-hooks.sh` wires `core.hooksPath` to `.githooks/`,
  enabling a `clang-format --dry-run -Werror` check on staged C sources at
  commit time.
- Fuzz:   `cmake --build build/fuzz --target fuzz_format fuzz_greet` then
  `./build/fuzz/tests/fuzz/fuzz_format tests/fuzz/corpus -dict=tests/fuzz/hello.dict -max_total_time=60`.

  Two harnesses cover the full public surface: `fuzz_format` exercises
  `hello_format` (the snprintf-into-buffer path) and `fuzz_greet` exercises
  `hello_greet` (the FILE*-streaming path). The library has no parser and
  the format string is a compile-time constant, so the input space these
  harnesses explore is bounded — but the harnesses exist so the *pattern* is
  visible when a future contributor adds a function that takes
  attacker-controlled bytes.
- CI:     see `.github/workflows/ci.yml` for the build/test matrix, sanitizer
  (ASan+UBSan, TSan), lint, coverage, fuzz, downstream-consumer, and Doxygen
  jobs.

## License

[MIT](LICENSE).
