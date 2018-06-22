load(
    "@pip_grpcio//:requirements.bzl",
    grpcio_requirement = "requirement",
)
load(
    "@local_pip//:requirements.bzl",
    requirement = "requirement",
)
load(
    "@io_bazel_rules_python//python:python.bzl",
    "py_binary",
    "py_library",
    "py_test",
)

filegroup(
    name = "fuzzer_configs",
    srcs = glob(["config/fuzzer/*.yaml"]),
    visibility = ["//:__subpackages__"],
)

py_library(
    name = "interceptor",
    srcs = ["interceptor.py"],
    data = [
        "test/build.sh",
        ":fuzzer_configs",
        "//build_system/preload_interceptor:preload_interceptor.so",
    ],
    deps = [
        requirement("pyyaml"),
    ],
)

py_binary(
    name = "intercept",
    srcs = ["main.py"],
    main = "main.py",
    deps = [
        ":interceptor",
    ],
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
        grpcio_requirement("setuptools"),
        grpcio_requirement("grpcio"),
        grpcio_requirement("protobuf"),
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
        grpcio_requirement("setuptools"),
        grpcio_requirement("grpcio"),
        grpcio_requirement("protobuf"),
    ],
)

py_test(
    name = "integration_test",
    size = "small",
    srcs = ["test/integration_test.py"],
    data = [
        "test/build.sh",
        "test/hello.c",
        "//build_system/preload_interceptor:preload_interceptor.so",
    ],
    imports = ["."],
    deps = [
        ":intercept",
        ":interceptor",
        "//build_system/interceptor:grpc_server",
        "//build_system/proto:py",
        grpcio_requirement("grpcio"),
        grpcio_requirement("protobuf"),
        grpcio_requirement("setuptools"),
    ],
)

sh_test(
    name = "lint",
    size = "small",
    srcs = ["test/lint.sh"],
    data = glob(["**/*.py"]),
)
