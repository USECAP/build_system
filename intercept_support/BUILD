cc_library(
    name = "matcher",
    srcs = ["matcher.cc"],
    hdrs = [
        "matcher.h",
        "types.h",
    ],
    deps = [
        "//build_system/proto:cpp",
        "@abseil//absl/strings",
        "@abseil//absl/types:optional",
        "@google_re2//:re2",
    ],
)

cc_library(
    name = "replacer",
    srcs = ["replacer.cc"],
    hdrs = [
        "cc_arg_info.h",
        "matcher.h",
        "replacer.h",
        "types.h",
    ],
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
        ":matcher",
        ":replacer",
        "@googletest//:gtest_main",
    ],
)
