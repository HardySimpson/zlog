# TEST

## tsan
to enable tsan test, build cmd:

```bash
# cd repo root
cmake -DCMAKE_BUILD_TYPE=Tsan -B build && cmake --build build -j8
ctest --test-dir build
```