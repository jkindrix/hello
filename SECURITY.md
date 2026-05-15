# Security Policy

## Supported versions

Only the latest minor release line receives security fixes.

| Version | Supported |
| ------- | --------- |
| 1.0.x   | ✅        |

## Reporting a vulnerability

Please **do not** open a public GitHub issue for suspected security problems.
Email the maintainers privately with:

- A description of the issue and its impact.
- Steps to reproduce, ideally a minimal program against the library.
- Affected version(s) and platform(s).

You will receive an acknowledgment within 72 hours. Once a fix is available,
we will coordinate a disclosure date with you and publish a patched release.

## Hardening

The project ships with the following hardening features enabled by default in
non-Debug builds:

- `_FORTIFY_SOURCE=2`
- `-fstack-protector-strong`
- `-fvisibility=hidden` for shared builds

CI exercises every change under AddressSanitizer + UndefinedBehaviorSanitizer.
