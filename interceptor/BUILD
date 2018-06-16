load("@pip_grpcio//:requirements.bzl", "requirement")
load(
    "@io_bazel_rules_python//python:python.bzl",
    "py_binary",
    "py_library",
    "py_test",
)

py_binary(
    name = "interceptor",
    srcs = ["main.py"],
    data = [
        "//build_system/preload_interceptor:preload_interceptor.so",
    ],
    main = "main.py",
)

py_library(
    name = "grpc_server",
    srcs = ["grpc_server.py"],
    visibility = [
        "//build_system:__pkg__",
        "//build_system/preload_interceptor:__pkg__",
    ],
    deps = [
        "//build_system/proto:py",
        requirement("setuptools"),
        requirement("grpcio"),
        requirement("protobuf"),
    ],
)

py_test(
    name = "grpc_test",
    size = "small",
    srcs = ["test/grpc_test.py"],
    data = [
        "test/build.sh",
    ],
    deps = [
        "//build_system/interceptor:grpc_server",
        "//build_system/proto:py",
        requirement("setuptools"),
        requirement("grpcio"),
        requirement("protobuf"),
    ],
)

py_test(
    name = "integration_test",
    size = "small",
    srcs = ["test/integration_test.py"],
    data = [
        "test/build.sh",
        "//build_system/preload_interceptor:preload_interceptor.so",
    ],
    imports = ["."],
    deps = [
        "//build_system/interceptor:grpc_server",
        "//build_system/proto:py",
        requirement("grpcio"),
        requirement("protobuf"),
        requirement("setuptools"),
    ],
)
