load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "protos",
    srcs = [
        "intercept.proto",
    ],
)

proto_library(
    name = "intercept_proto",
    srcs = ["intercept.proto"],
)

cc_grpc_library(
    name = "cpp",
    srcs = [":protos"],
    proto_only = False,
    well_known_protos = [],
    deps = [],
)

go_proto_library(
    name = "intercept_go_proto",
    compilers = ["@io_bazel_rules_go//proto:go_grpc"],
    importpath = "gitlab.com/code-intelligence/core/build_system/proto",
    proto = ":intercept_proto",
)

go_library(
    name = "go_default_library",
    embed = [":intercept_go_proto"],
    importpath = "gitlab.com/code-intelligence/core/build_system/proto",
)
