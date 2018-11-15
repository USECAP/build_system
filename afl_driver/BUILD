load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

filegroup(
    name = "afl_driver",
    srcs = [
        "afl_driver.cpp",
    ],
    visibility = ["//visibility:public"],
)

go_test(
    name = "test",
    srcs = [
        "afl_driver_test.go",
    ],
    data = [
        ":testdata",
        "@afl",
    ],
    deps = [
        "//utils/pathutils:go_default_library",
        "@com_github_bazelbuild_rules_go//go/tools/bazel:go_default_library",
    ],
)

filegroup(
    name = "testdata",
    srcs = glob(["testdata/**"]),
)
