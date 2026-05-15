# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- ThreadSanitizer CI job to keep the `tsan` preset honest. Single-threaded
  today, but future-proofs the harness.
- Fuzz seed corpus under `tests/fuzz/corpus/` (empty input, ASCII names,
  spaces, embedded NULs, long inputs, UTF-8). The CI fuzz job now starts
  the fuzzer pointed at the corpus instead of mutating from scratch.
- Git pre-commit hook at `.githooks/pre-commit` plus
  `scripts/install-hooks.sh` to wire `core.hooksPath`. Runs
  `clang-format --dry-run -Werror` on staged C sources locally so the
  feedback loop is shorter than waiting for the lint CI job.
- README: a "Consuming on Windows / MSVC" section covering static vs
  shared linkage, `HELLO_USE_SHARED`, `/fsanitize=address`, and how the
  Linux-specific hardening flags are skipped on MSVC.
- README: a CLI exit-code table (0 / 1 / 2 semantics) for scripting
  consumers.
- Doxygen `@file` blocks on `src/hello.c` and `src/main.c`.

- `pkg-config` integration: a relocatable `hello.pc` is generated and
  installed to `<libdir>/pkgconfig`. The file uses `${pcfiledir}` so it works
  after the install tree is moved.
- Downstream consumer test under `tests/consumer/`: a minimal external
  project that `find_package(hello)`-s and links `hello::hello`. A new
  `install-consumer` CI job installs the library to a staging prefix,
  verifies `pkg-config --cflags --libs hello`, and builds + runs the
  consumer end-to-end.
- libFuzzer harness under `tests/fuzz/` plus a `HELLO_BUILD_FUZZERS` CMake
  option (Clang-only) and a `fuzz` CI job that runs `fuzz_format` for ~20s
  on every push.
- `.github/dependabot.yml` to keep CI actions current.
- `.github/CODEOWNERS`, `PULL_REQUEST_TEMPLATE.md`, and issue templates
  (bug, feature, security-disclosure pointer).
- README status badges (CI, license, C standard, CMake version) and a
  "Consuming from non-CMake build systems" section.

### Fixed
- CTest CLI smoke-test regexes now accept `\r?\n` so they pass on Windows
  text-mode stdout (previously anchored on bare `\n`, which never matched
  `\r\n`).

### Added
- New CLI smoke test that exercises multiple positional NAME arguments.

### Changed
- CLI: `--help` now propagates stdout I/O failures with exit code 2, matching
  `--version`'s behavior.
- Hardening: auto-detect glibc `_FORTIFY_SOURCE=3` support and prefer it over
  `=2` in non-Debug builds.
- Hardening: link executables on Linux with `-pie`, `-Wl,-z,relro`,
  `-Wl,-z,now`, `-Wl,-z,noexecstack`.
- CI: drop the `ctest --preset default` invocation on the release matrix leg
  (the preset only targets Debug) and just call `ctest` directly.
- CI: add a `docs` job that builds the Doxygen site and uploads it as an
  artifact.
- CMake: drop the placeholder `HOMEPAGE_URL`.
- SECURITY.md: document the actual hardening set (FORTIFY=3 with =2 fallback,
  PIE, full RELRO, BIND_NOW, noexecstack) instead of the stale FORTIFY=2 list.
- README: clarify that the CI bullet refers to GitHub Actions, not git hooks.

## [1.0.0] - 2026-05-15

### Added
- Public C API: `hello_greet`, `hello_format`, `hello_version`,
  `hello_status_string`.
- `hello` CLI with `--help` / `--version` and multi-name greetings.
- Modern CMake build with install/export, `CMakePresets.json`, version header
  generation, and `find_package(hello)` support.
- Sanitizer (ASan/UBSan/TSan/MSan) and coverage build presets.
- Unit tests and CLI smoke tests via CTest.
- GitHub Actions CI: GCC + Clang, Debug + Release + sanitizers, with
  `clang-format` and `clang-tidy` checks.
- Editor/formatter/linter configs: `.editorconfig`, `.clang-format`,
  `.clang-tidy`, `.gitattributes`.
