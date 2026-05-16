# Security Policy

## Supported versions

Only the latest minor release line receives security fixes.

| Version | Supported |
| ------- | --------- |
| 1.1.x   | ✅        |
| 1.0.x   | ❌        |

## Reporting a vulnerability

Please **do not** open a public GitHub issue for suspected security problems.
Email the maintainers privately with:

- A description of the issue and its impact.
- Steps to reproduce, ideally a minimal program against the library.
- Affected version(s) and platform(s).

You will receive an acknowledgment within 72 hours. Once a fix is available,
we will coordinate a disclosure date with you and publish a patched release.

## Hardening

The project ships with the following hardening features:

Compile-time (non-Debug builds):
- `_FORTIFY_SOURCE=3` when the toolchain supports it (glibc ≥ 2.34, GCC ≥ 12
  or Clang ≥ 9), with automatic fallback to `_FORTIFY_SOURCE=2`. Detection is
  done at configure time via `check_c_source_compiles`.
- `-fstack-protector-strong`.
- `-fvisibility=hidden` plus an explicit `HELLO_API` export macro, so only
  intentionally-public symbols are visible from shared builds.

Link-time (Linux executables):
- `-pie` (position-independent executables).
- `-Wl,-z,relro -Wl,-z,now` (full RELRO + immediate symbol binding).
- `-Wl,-z,noexecstack` (non-executable stack).

CI exercises every change under AddressSanitizer + UndefinedBehaviorSanitizer
with `halt_on_error=1` and `detect_leaks=1`.
