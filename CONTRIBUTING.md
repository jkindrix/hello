# Contributing

Thanks for considering a contribution! This project is small but its standards
are not. Please read this before opening a pull request.

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
