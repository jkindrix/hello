# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Test harness gains a `REQUIRE(cond)` macro that short-circuits the
  enclosing test on failure. Replaces the latent crash risk in
  `greet_writes_expected_bytes` and `status_string_covers_all_codes`,
  where a contract regression would have crashed the whole harness
  instead of producing a clean FAIL diagnostic.
- Sanitizer runtime options are now baked into CTest properties (via
  `set_property(TEST ... APPEND PROPERTY ENVIRONMENT ...)` keyed on
  `HELLO_ENABLE_ASAN` / `_UBSAN` / `_TSAN` / `_MSAN`). Local
  `ctest --preset asan` now sees the same strictness as the CI job; the
  CI env blocks no longer disagree with the local surface.
- MemorySanitizer CI job (Clang-only). Closes the gap where
  `HELLO_ENABLE_MSAN` was wired but never exercised.
- Second fuzz harness `fuzz_greet` exercising the `hello_greet`
  streaming path; `LLVMFuzzerInitialize` opens the null device once and
  the harness reuses it. The full public surface is now fuzzed.
- libFuzzer dictionary `tests/fuzz/hello.dict` containing the format
  string's literal tokens, so the mutator can splice them directly.
  Both fuzz CI jobs pass `-dict=tests/fuzz/hello.dict`.
- Doc-coverage policy test `tests/check_doxygen_coverage.sh` plus a
  CTest entry. Asserts that every `HELLO_API` declaration in
  `include/hello/hello.h` is preceded by a Doxygen `/** ... */` block.
  Same shape as `check_exports.sh` — the contract is in writing and the
  test makes drift visible immediately.
- `CODE_OF_CONDUCT.md` adopting Contributor Covenant v2.1 by reference,
  with private-reporting and enforcement-ladder pointers.
- `SUPPORT.md` mapping question type to channel (issues vs. Discussions
  vs. private security advisory).
- `CONTRIBUTING.md` "Governance" section stating single-maintainer
  policy, merge requirements, review SLA, and release cadence.
- `.github/ISSUE_TEMPLATE/config.yml` adds Discussions and SUPPORT links
  alongside the existing security-advisory redirect.
- README "Example output" block under "Build & run".
- README "Troubleshooting" table covering the five common consumer
  failure modes (`find_package` not found, MSVC shared linkage without
  `HELLO_USE_SHARED`, missing `PKG_CONFIG_PATH`, sanitizer/ASLR
  collision, missing Clang runtime package).
- README fuzz section now describes both harnesses and explicitly notes
  why the input space they explore is small (no parser; format string
  is a compile-time constant).
- CLI `--` end-of-options terminator so names beginning with `-` can be
  greeted (`hello -- --version` greets the literal name `--version`).
  Covered by two new CTest cases.
- Portable `HELLO_ERR_IO` test (`greet_reports_io_error_on_read_only_stream`)
  that opens the platform null device in read mode and asserts
  `hello_greet` reports `HELLO_ERR_IO`. Complements the POSIX-only
  broken-pipe test and closes the Windows coverage gap for that branch.
- CLI `--help` output now lists the `-h` and `-V` short aliases (they
  were already accepted, just undocumented).
- Unit test for the `HELLO_ERR_IO` branch of `hello_greet`, driving
  `fprintf` failure via a pipe with its read end closed and `SIGPIPE`
  ignored. POSIX-guarded; skipped on Windows.
- CodeQL workflow (`security-and-quality` queries, weekly schedule).
- Coverage CI job now enforces a 90 % line-coverage floor; the artifact
  upload still runs for inspection.
- Doxygen site is published to GitHub Pages on push to `main`. PRs build
  docs as an artifact but do not deploy.
- `symbol_export_check` CTest test (Linux + `BUILD_SHARED_LIBS=ON` only)
  plus a `shared-build` CI job. Asserts that `libhello.so`'s dynamic
  symbol table is exactly the four documented public functions.
- ThreadSanitizer CI job to keep the `tsan` preset honest.
- Fuzz seed corpus under `tests/fuzz/corpus/` (empty, ASCII, spaces,
  embedded NUL, long input, UTF-8). The fuzz CI job now starts the
  fuzzer pointed at the corpus instead of mutating from scratch.
- Git pre-commit hook at `.githooks/pre-commit` plus
  `scripts/install-hooks.sh` to wire `core.hooksPath`. Runs
  `clang-format --dry-run -Werror` on staged C sources locally.
- `pkg-config` integration: a relocatable `hello.pc` (uses
  `${pcfiledir}`) is generated and installed to `<libdir>/pkgconfig`.
- Downstream consumer test under `tests/consumer/`. A new
  `install-consumer` CI job installs the library to a staging prefix,
  verifies `pkg-config --cflags --libs hello`, and builds + runs the
  consumer end-to-end.
- libFuzzer harness under `tests/fuzz/` plus a `HELLO_BUILD_FUZZERS`
  CMake option (Clang-only) and a `fuzz` CI job that runs `fuzz_format`
  for ~20 s on every push.
- `.github/dependabot.yml` to keep CI actions current.
- `.github/CODEOWNERS`, `PULL_REQUEST_TEMPLATE.md`, and issue templates
  (bug, feature, security-disclosure pointer).
- New CLI smoke test exercising multiple positional NAME arguments.
- README: status badges (CI, CodeQL, license, C standard, CMake
  version), CLI exit-code table, "Consuming on Windows / MSVC" section,
  "Consuming from non-CMake build systems" section, versioning &
  ABI-stability policy, and a link to the published Doxygen site.
- README: Doxygen `@file` blocks on `src/hello.c` and `src/main.c`.
- CONTRIBUTING: one-time maintainer-setup note covering the
  GitHub-Pages source toggle and CodeQL enablement.

### Changed
- Split the `docs` workflow job into a build-only `docs` job (runs on
  every trigger, uploads the artifact) and a deploy-only `pages` job
  (push-to-main only, owns the `pages`/`id-token` permissions and the
  `github-pages` environment). PRs no longer carry deploy permissions
  or surface an empty Pages URL.
- CodeQL job builds with `-G Ninja` instead of Unix Makefiles and with
  `-DHELLO_WARNINGS_AS_ERRORS=OFF` so a future CodeQL-injected warning
  cannot mask the analyzer findings by failing the build.
- `HELLO_BUILD_FUZZERS=ON` on a non-Clang toolchain now emits
  `message(WARNING)` and auto-disables the option instead of failing
  configure with `FATAL_ERROR`.
- CLI: `--help` now propagates stdout I/O failures with exit code 2,
  matching `--version`'s behavior.
- Hardening: auto-detect glibc `_FORTIFY_SOURCE=3` support and prefer
  it over `=2` in non-Debug builds.
- Hardening: link executables on Linux with `-pie`, `-Wl,-z,relro`,
  `-Wl,-z,now`, `-Wl,-z,noexecstack`.
- CI: drop the `ctest --preset default` invocation on the release
  matrix leg (the preset only targets Debug); call `ctest` directly.
- CMake: drop the placeholder `HOMEPAGE_URL`.
- SECURITY.md: document the actual hardening set (FORTIFY=3 with =2
  fallback, PIE, full RELRO, BIND_NOW, noexecstack) instead of the
  stale FORTIFY=2 list.
- Coverage threshold extraction now anchors `^ *lines\.+:` instead of
  the ambiguous `lines\.*:`, and reports an explicit error if the
  percentage cannot be parsed.

### Fixed
- **CLI's documented exit-code 2 for I/O errors only held when the caller
  masked `SIGPIPE`.** In a real shell pipeline (`hello Ada | head -n 0`)
  the kernel killed the writer with `SIGPIPE` and the shell reported
  exit 141. `main()` now installs `signal(SIGPIPE, SIG_IGN)` on POSIX
  (guarded by `#ifdef SIGPIPE` so Windows builds are unaffected), so
  the broken-pipe path traverses `fprintf → -1`, `HELLO_ERR_IO`, exit 2
  as the README claims. The `cli_broken_pipe_exit_code` CTest entry
  no longer pre-masks `SIGPIPE` either, so it now exercises the same
  scenario a scripting consumer would.

### Added
- `scripts/coverage.sh`: version-aware local coverage runner. Auto-
  detects lcov 1.x vs 2.x and applies the matching flag set so
  contributors on older distros (Debian 12 still ships lcov 1.16) can
  reproduce the CI coverage gate without hand-editing the workflow
  commands. Enforces the same 90 % line-coverage floor.

### Fixed (continued)
- **CLI exit-code contract was wrong for library-surfaced I/O errors.**
  `hello_greet` returning `HELLO_ERR_IO` previously caused the CLI to
  exit 1, but the README's exit-code table documented 2 for "I/O error
  writing to stdout". `main()` now maps `HELLO_ERR_IO` → exit 2,
  matching the contract. Verified via a new `cli_broken_pipe_exit_code`
  test that closes the pipe reader and asserts the CLI exits 2.
- **Coverage CI gate was failing its own threshold.** Filtered line
  coverage was 87.0 % against a 90 % floor, so the coverage job would
  have failed on first push. Two new tests (`cli_unknown_option` and
  the broken-pipe test above) plus refactoring the greet-and-exit logic
  into a single helper push filtered coverage to 97.1 % (hello.c 97.3 %,
  main.c 97.0 %). Verified locally with lcov.
- **Doxygen builds were silently emitting an unresolved-reference
  warning.** `WARN_AS_ERROR` was `NO` and the Doxyfile's `INPUT` list
  didn't include the project's markdown files, so README cross-links
  to `CONTRIBUTING.md` couldn't resolve. The Doxyfile now lists
  CONTRIBUTING / CODE_OF_CONDUCT / SECURITY / SUPPORT / CHANGELOG and
  sets `WARN_AS_ERROR=YES` so future drift fails the docs job.
- `@jkindrix` GitHub mentions in CONTRIBUTING.md and CODE_OF_CONDUCT.md
  are now wrapped in backticks so Doxygen doesn't interpret them as
  commands (it parses bare `@name` as `\name`).

- CTest CLI smoke-test regexes now accept `\r?\n` so they pass on
  Windows text-mode stdout (previously anchored on bare `\n`).

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
