cc_binary(
    name = "preload_interceptor.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkopts = [
        "-ldl",
    ],
    linkshared = 1,
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//build_system/proto:cpp",
        "//build_system/replacer",
        "@abseil//absl/types:optional",
    ],
)
