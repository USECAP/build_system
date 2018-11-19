filegroup(
    name = "libllvm_coverage.a",
    srcs = [
        "llvm_coverage",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "llvm_coverage",
    srcs = [
        "StandaloneFuzzTargetMain.c",
    ],
    copts = ["-std=c99"],
    linkstatic = 1,
    visibility = ["//visibility:public"],
)
