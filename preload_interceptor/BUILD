load("@pip_grpcio//:requirements.bzl", "requirement")

cc_binary(
    name = "preload_interceptor.so",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    linkopts = [
        "-ldl",
        "-lstdc++fs",
    ],
    linkshared = 1,
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        "//build_system/intercept_support",
        "//build_system/proto:cpp",
        "@abseil//absl/types:optional",
    ],
)
