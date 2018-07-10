load("@io_bazel_rules_go//go:def.bzl", "go_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

package(default_visibility = ["//visibility:public"])

load("@org_pubref_rules_protobuf//cpp:rules.bzl", "cc_proto_library")
load("@org_pubref_rules_protobuf//python:rules.bzl", "py_proto_library")

filegroup(
    name = "protos",
    srcs = [
        "intercept.proto",
    ],
)

cc_proto_library(
    name = "cpp",
    protos = [":protos"],
    verbose = 0,
    with_grpc = True,
)

py_proto_library(
    name = "py",
    protos = [":protos"],
    verbose = 0,
    with_grpc = True,
)

proto_library(
    name = "intercept_proto",
    srcs = ["intercept.proto"],
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
