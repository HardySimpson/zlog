# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

**zlog** 是一个高性能、线程安全、纯C语言日志库（C99标准），版本 1.2.18，Apache 2.0 许可。支持 Linux、macOS、AIX、Windows，无第三方依赖（仅 POSIX + pthread）。

## 构建命令

### CMake（推荐）

```bash
# 基本构建
cmake -B build
cmake --build build -j8
cmake --install build

# 带单元测试
cmake -DUNIT_TEST=ON -B build
cmake --build build -j8
ctest --test-dir build           # 运行所有测试
ctest --test-dir build -V        # 详细输出
ctest --test-dir build -R <name> # 运行单个测试

# ThreadSanitizer 构建
cmake -DCMAKE_BUILD_TYPE=Tsan -B build
```

### Makefile（传统）

```bash
make                   # 构建动态库
make install           # 安装（默认 /usr/local）
make PREFIX=/opt/zlog install
make 32bit             # 32位构建
make test              # 构建并运行测试
make clean
```

### 配置验证工具

```bash
./src/zlog-chk-conf <config_file>
```

## 架构概览

数据流：**初始化** → **配置解析** → **分类匹配** → **规则过滤** → **格式化** → **输出目标**

### 核心组件

| 模块 | 文件 | 职责 |
|------|------|------|
| 核心库 | `src/zlog.c` | 全局状态、init/fini、公共 API |
| 配置系统 | `src/conf.c` | 配置文件解析、验证、重加载 |
| 分类系统 | `src/category.c/h`, `src/category_table.c` | 日志分类、规则关联、级别位图 |
| 规则引擎 | `src/rule.c` | 规则匹配、输出分发、文件轮转触发 |
| 格式系统 | `src/format.c`, `src/spec.c` | 转换字符解析、消息格式化 |
| 线程系统 | `src/thread.c`, `src/consumer.c`, `src/fifo.c` | TLS 管理、异步输出、生产者/消费者 |
| 缓冲系统 | `src/buf.c` | 动态缓冲，可自适应大小 |
| 文件轮转 | `src/rotater.c`, `src/lockfile.c` | 大小触发轮转、进程间锁 |
| MDC | `src/mdc.c` | Mapped Diagnostic Context（线程上下文数据） |
| 工具库 | `src/zc_arraylist.c`, `src/zc_hashtable.c`, `src/zc_util.c` | 内部数据结构 |

### 线程安全机制

- `pthread_rwlock_t` 保护全局配置
- TLS（线程局部存储）存储线程相关数据（缓冲区、事件等）
- 文件轮转使用锁文件（`src/lockfile.c`）保证进程间同步

### 公共 API（`src/zlog.h`）

```c
zlog_init(config_path)          // 初始化
zlog_get_category("cat_name")   // 获取分类句柄
zlog_info(cat, fmt, ...)        // 普通日志宏（含 debug/warn/error/fatal）
zlog_fini()                     // 清理

// 默认分类接口
dzlog_init(config_path, "cat_name")
dzlog_info(fmt, ...)

// 运行时重加载
zlog_reload(config_path)
```

## 配置文件格式

```ini
[global]
strict_init = 1
buf_size_min = 1K
buf_size_max = 16M

[formats]
simple = "%m%n"
default = "%d(%F %T).%ms %-6V (%c:%F:%L) - %m%n"

[rules]
my_cat.INFO   > /var/log/app.log, 100M; default
my_cat.*      > stdout;           simple
```

**常用转换字符：** `%m`(消息) `%n`(换行) `%d`(时间) `%t`(线程ID) `%c`(分类) `%V`(级别) `%F`(文件) `%L`(行号) `%M`(函数名)

**输出目标：** `>stdout` `>stderr` `>/path/file, SIZE` `>|program`（管道） `>sys; facility`（syslog）

## 测试

测试文件位于 `test/`，共 30 个 C 测试程序。集成测试脚本：`scripts/test.sh`。

配置示例参考 `test/test_hello.conf`。

## 平台注意事项

- macOS：生成 `.dylib`，Makefile 自动适配
- Windows：需引入 `unixem` 库，CMake 条件处理
- C++ 用户可使用 `src/zlog_cpp.h` 包装器（ZlogCPP 类 + ZLOGD/I/W/E 宏）
