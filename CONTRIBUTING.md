# Contributing

Thanks for considering a contribution! This project is small but its standards
are not. Please read this before opening a pull request.

## One-time maintainer setup

Three repository settings have to be flipped by hand the first time you
publish this template; the CI pipeline and the issue/support workflows
assume all three are in place:

- **Settings → Pages → Source: GitHub Actions.** The `pages` job in
  `.github/workflows/ci.yml` deploys the Doxygen site on every push to
  `main` and will fail until this is set.
- **Settings → Code security → Code scanning: enable CodeQL.** Required
  before `.github/workflows/codeql.yml` can upload SARIF results.
- **Settings → General → Features → Discussions: enable.**
  [SUPPORT.md](SUPPORT.md) and `.github/ISSUE_TEMPLATE/config.yml` both
  redirect how-to / design questions to Discussions; the links 404 until
  the feature is on.

## Governance

This is a single-maintainer project ([@jkindrix](https://github.com/jkindrix)).
Pull requests are merged when:

1. CI is green on every required job (the full workflow under
   `.github/workflows/ci.yml` plus the CodeQL workflow).
2. The maintainer has approved the PR.
3. There are no unresolved review threads.

The maintainer reviews PRs on a best-effort basis; expect a first response
within a week, occasionally longer for non-trivial changes. Pinging once
after seven quiet days is welcome. There is no formal release cadence:
releases are cut when the `[Unreleased]` section of [CHANGELOG.md](CHANGELOG.md)
accumulates a meaningful set of changes, or when a critical fix lands.

Conduct in any project space (issues, PRs, discussions) is governed by the
[Code of Conduct](CODE_OF_CONDUCT.md).

## Ground rules

- **Discuss before large changes.** Open an issue first for anything beyond a
  bug fix or doc tweak.
- **Keep changes focused.** One logical change per PR. Mixed PRs are hard to
  review and risk reverting unrelated work.
- **Write tests** for new behavior or bug fixes. CI must be green.
- **Follow the style.** Run `scripts/format.sh` and `scripts/lint.sh` before
  pushing.

## Building & testing

```sh
cmake --preset debug
cmake --build --preset debug
ctest --preset default
```

Run the sanitizer build at least once before opening a PR:

```sh
cmake --preset asan && cmake --build --preset asan
ctest --preset asan
```

### Sanitizers on newer Linux kernels (WSL2, kernel ≥ 6.x)

Linux ≥ 6.x defaults `vm.mmap_rnd_bits` to 32, which collides with the shadow
memory layout used by Clang ≤ 15's ASan/TSan/libFuzzer runtimes. You'll see
either `FATAL: ThreadSanitizer: unexpected memory mapping` or sporadic
SEGFAULTs from sanitizer-instrumented binaries. Either:

- **Use Clang ≥ 16** for sanitizer/fuzz builds (recommended; the runtime was
  fixed there). On Debian/Ubuntu:

  ```sh
  sudo apt install -y clang-19 clang-tools-19 libclang-rt-19-dev
  CC=clang-19 cmake --preset asan
  CC=clang-19 cmake --preset tsan
  CC=clang-19 cmake -S . -B build/fuzz -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug -DHELLO_BUILD_FUZZERS=ON
  ```

- **Or lower the ASLR entropy at runtime** (doesn't persist across reboots):

  ```sh
  sudo sysctl -w vm.mmap_rnd_bits=28
  ```

CI's `ubuntu-latest` runners use a kernel/clang combination where this isn't
an issue.

## Coding standards

- C17 (no extensions: `-std=c17`, `CMAKE_C_EXTENSIONS=OFF`).
- All public symbols declared in `include/hello/`; private helpers `static`.
- All allocations checked; pointers `NULL`-ed after `free` if they remain live.
- All sized buffers carry an explicit size; no unchecked `strcpy`/`sprintf`.
- Warnings are errors. Do not silence; fix.
- Functions get short, plain-English Doxygen comments only where the API
  contract is non-obvious. Implementation files: no narration comments.

## Commit messages

Use [Conventional Commits](https://www.conventionalcommits.org/):

```
feat(cli): support reading names from stdin
fix(format): handle SIZE_MAX correctly in length-query path
docs(readme): add cross-compilation note
test(greet): add tmpfile-based stream test
refactor(api): rename HELLO_OK alias
chore(ci): bump actions/checkout to v4
```

Keep the subject ≤ 72 characters; explain *why* in the body when not obvious.

## Pull-request checklist

- [ ] Builds clean under `cmake --preset debug` and `--preset release`.
- [ ] `ctest --preset default` passes.
- [ ] `ctest --preset asan` passes (no ASan/UBSan diagnostics).
- [ ] `scripts/format.sh` is a no-op (code is already formatted).
- [ ] `scripts/lint.sh` reports no new warnings.
- [ ] New behavior is documented in `README.md` and/or header comments.
- [ ] `CHANGELOG.md` updated under an `## [Unreleased]` heading.
