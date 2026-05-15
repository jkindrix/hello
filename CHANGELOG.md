# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
