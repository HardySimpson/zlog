# TEST

The unit tests are only built when `UNIT_TEST` is on:

```bash
# cd repo root
cmake -DUNIT_TEST=ON -B build && cmake --build build -j8
ctest --test-dir build
```

## tsan

Thread sanitizer, to catch data races:

```bash
cmake -DCMAKE_BUILD_TYPE=Tsan -DUNIT_TEST=ON -B build && cmake --build build -j8
ctest --test-dir build
```

## asan

Address sanitizer, to catch invalid accesses and leaks:

```bash
cmake -DCMAKE_BUILD_TYPE=Asan -DUNIT_TEST=ON -B build && cmake --build build -j8
ctest --test-dir build
```

Both build types need gcc or clang; the configure step fails on any other
compiler rather than quietly building without a sanitizer.

# RELEASE

The version number lives in exactly one place, `src/version.h`:

```c
#define ZLOG_VERSION "1.2.19"
```

Both build systems parse that line — CMake in `CMakeLists.txt` (it also feeds
`CPACK_PACKAGE_VERSION` and the soname) and make in `src/Makefile` — so nothing
else in the tree needs editing, and both fail loudly if the line cannot be
parsed. The soname is `major.minor`, so bumping the minor version breaks the
ABI contract for already linked programs; bump it only when that is intended.

Steps for a release:

1. Bump `ZLOG_VERSION` in `src/version.h`.
2. Add the matching entry at the top of `Changelog`.
3. Commit both, e.g. `version: 1.2.19`.
4. Check that the build agrees:

   ```bash
   cmake -B build            # prints: version : 1.2.19 (soname 1.2)
   ```

5. Tag the commit, using the bare version number as the tag name — no `v`
   prefix, matching the existing tags:

   ```bash
   git tag -a 1.2.19 -m "version: 1.2.19"
   ```

6. Push the commit and the tag:

   ```bash
   git push origin master
   git push origin 1.2.19
   ```

Always bump `src/version.h` *before* tagging: the tag has to point at a commit
that already carries the new version, otherwise the tarball GitHub generates
for the release reports the previous version.

The `Version:` field in `zlog.spec` is not derived from `src/version.h` and is
maintained separately; keep it in mind if you build RPMs from that spec file
rather than with CPack.
