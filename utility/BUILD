cc_library(
    name = "utility",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.h"]),
    visibility = ["//visibility:public"],
    deps = [
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
    linkopts = [
        "-lstdc++fs",
    ],
    deps = [
        ":utility",
        "@googletest//:gtest_main",
    ],
)
