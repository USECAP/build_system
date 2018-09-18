cc_binary(
    name = "preload_interceptor.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkopts = [
        "-ldl",
    ],
    linkshared = True,
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = [
        "//build_system/proto:cpp",
        "//build_system/replacer",
        "@abseil//absl/types:optional",
        "@com_github_grpc_grpc//:grpc++_unsecure",
    ],
)
