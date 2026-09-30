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

The version number lives in exactly one place, `src/version.h`, as a single
`#define ZLOG_VERSION "MAJOR.MINOR.PATCH"` line.

Both build systems parse that line — CMake in `CMakeLists.txt` (it also feeds
`CPACK_PACKAGE_VERSION` and the soname) and make in `src/Makefile` — so nothing
else in the tree needs editing, and both fail loudly if the line cannot be
parsed. The soname is `major.minor`, so bumping the minor version breaks the
ABI contract for already linked programs; bump it only when that is intended.

Steps for a release:

1. Bump `ZLOG_VERSION` in `src/version.h`, and the `Version:` field in
   `zlog.spec`, which is not derived from it.
2. Add the matching entry at the top of `Changelog`.
3. Commit both, with the new version as the subject: `version: $(zlog_version)`
   where the shell function below reads it back out of the tree.
4. Check that the build agrees — it prints the version and the soname it
   derived:

   ```bash
   cmake -B build            # prints: version : MAJOR.MINOR.PATCH (soname MAJOR.MINOR)
   ```

5. Tag the commit, using the bare version number as the tag name — no `v`
   prefix, matching the existing tags.
6. Push the commit, then the tag.

Steps 3, 5 and 6 read the version rather than repeating it, so there is nothing
to keep in step with a release:

```bash
zlog_version() { sed -n 's/^#define ZLOG_VERSION "\(.*\)"/\1/p' src/version.h; }

git commit -m "version: $(zlog_version)" src/version.h zlog.spec Changelog
git tag -a "$(zlog_version)" -m "version: $(zlog_version)"
git push origin master
git push origin "$(zlog_version)"
```

Always bump `src/version.h` *before* tagging: the tag has to point at a commit
that already carries the new version, otherwise the tarball GitHub generates
for the release reports the previous version.
