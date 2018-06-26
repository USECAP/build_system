cc_library(
    name = "intercept_support",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    linkopts = [
        "-lstdc++fs",
    ],
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
        ":intercept_support",
        "@googletest//:gtest_main",
    ],
)
