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
