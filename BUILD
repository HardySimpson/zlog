load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "zlog",
    srcs = glob(
        [
            "src/*.c",
            "src/*.h",
        ],
        exclude = [
            "src/zlog_win.c",
            "src/zlog_win.h",
        ],
    ),
    hdrs = glob(
        [
            "src/*.h",
            "*.h",
        ],
        exclude = [
            "src/zlog_win.h",
        ],
    ),
    visibility = ["//visibility:public"],
)
