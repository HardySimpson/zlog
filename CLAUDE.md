# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

**zlog** is a high performance, thread safe, pure C logging library (C99), version 1.2.20, under the Apache 2.0 licence. It supports Linux, macOS, AIX and Windows, and has no third party dependencies (POSIX + pthread only).

## Build commands

### CMake (preferred)

```bash
# plain build
cmake -B build
cmake --build build -j8
cmake --install build

# with the unit tests
cmake -DUNIT_TEST=ON -B build
cmake --build build -j8
ctest --test-dir build           # run every test
ctest --test-dir build -V        # verbose output
ctest --test-dir build -R <name> # run a single test

# ThreadSanitizer build
cmake -DCMAKE_BUILD_TYPE=Tsan -B build
```

### Makefile (traditional)

```bash
make                   # build the shared library
make install           # install (/usr/local by default)
make PREFIX=/opt/zlog install
make 32bit             # 32 bit build
make test              # build and run the tests
make clean
```

### Configuration checker

```bash
./src/zlog-chk-conf <config_file>
```

## Architecture

Data flow: **init** → **configuration parsing** → **category matching** → **rule filtering** → **formatting** → **output**

### Core components

| Module | Files | Responsibility |
|--------|-------|----------------|
| Core | `src/zlog.c` | global state, init/fini, public API |
| Configuration | `src/conf.c` | parsing, validation, reload |
| Categories | `src/category.c/h`, `src/category_table.c` | log categories, rule association, level bitmap |
| Rule engine | `src/rule.c` | rule matching, output dispatch, rotation trigger |
| Formatting | `src/format.c`, `src/spec.c` | conversion character parsing, message formatting |
| Threading | `src/thread.c`, `src/consumer.c`, `src/fifo.c` | TLS management, asynchronous output, producer/consumer |
| Buffers | `src/buf.c` | dynamic buffer, grows as needed |
| Rotation | `src/rotater.c`, `src/lockfile.c` | size triggered rotation, inter-process lock |
| MDC | `src/mdc.c` | Mapped Diagnostic Context (per thread context data) |
| Utilities | `src/zc_arraylist.c`, `src/zc_hashtable.c`, `src/zc_util.c` | internal data structures |

### Thread safety

- `pthread_rwlock_t` protects the global configuration
- TLS (thread local storage) holds the per thread data (buffers, events, ...)
- rotation uses a lock file (`src/lockfile.c`) to synchronise between processes

### Public API (`src/zlog.h`)

```c
zlog_init(config_path)          // initialise
zlog_get_category("cat_name")   // get a category handle
zlog_info(cat, fmt, ...)        // the usual macros (debug/warn/error/fatal too)
zlog_fini()                     // clean up

// default category interface
dzlog_init(config_path, "cat_name")
dzlog_info(fmt, ...)

// reload at run time
zlog_reload(config_path)
```

## Configuration file format

```ini
[global]
strict init = true
buffer min = 1K
buffer max = 16M
rotate lock file = /tmp/zlog-rotate.lock

[formats]
simple = "%m%n"
default = "%d(%F %T).%ms %-6V (%c:%F:%L) - %m%n"

[rules]
my_cat.INFO   "/var/log/app.log", 100M; default
my_cat.*      >stdout; simple
```

The `[global]` keys are space separated words (`strict init`, `buffer min`,
`buffer max`, `file perms`, `rotate lock file`, `default format`,
`reload conf period`, `fsync period`), not underscore names. `rotate lock file`
defaults to the configuration file itself, which `src/lockfile.c:28` opens
`O_RDWR`; when that file is not writable, zlog falls back to `/tmp/zlog.lock`
and warns (see `zlog_conf_fallback_rotate_lock_file` in `src/conf.c`). A lock
file named in `[global]` is never replaced that way.

**Common conversion characters:** `%m` (message) `%n` (newline) `%d` (time)
`%ms`/`%us` (milli/microseconds) `%t` (thread id) `%c` (category)
`%V`/`%v` (level, upper/lower case) `%F`/`%f` (source file, full path/basename)
`%L` (line) `%U` (function) `%p` (pid) `%H` (hostname) `%M(key)` (MDC value)

**Output targets:** `"/path/file", SIZE` (a file, the path must be quoted)
`>stdout` `>stderr` `>syslog, LOG_LOCAL0` (syslog) `| program` (pipe)

Note that `>` is only for `stdout`, `stderr` and `syslog`: a file target needs a
quoted path, and both `>` followed by a path and an unquoted path are rejected
by `src/rule.c` (`the string after is not syslog, stdout or stderr`).

## Tests

The tests live in `test/`: 38 C programs, plus a fuzzer under `test/fuzzers/`.
The integration script is `scripts/test.sh`.

See `test/test_hello.conf` for a configuration example.

## Platform notes

- macOS: builds a `.dylib`, the makefile adapts on its own
- Windows: needs the `unixem` library, handled conditionally in CMake
- C++: the library is pure C, with no C++ code and no C++ wrapper classes; `src/zlog.h` is guarded with `extern "C"`, so it can be included from C++ directly
