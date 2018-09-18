cc_library(
    name = "replacer",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
        "//build_system/proto:cpp",
        "@abseil//absl/strings",
        "@abseil//absl/types:optional",
        "@google_re2//:re2",
    ],
)

cc_test(
    name = "test",
    size = "small",
    srcs = glob(["test/*_test.cc"]),
    copts = ["-Iexternal/gtest/include"],
    deps = [
        ":replacer",
        "@com_github_grpc_grpc//:grpc++_unsecure",
        "@googletest//:gtest_main",
    ],
)
