load("@local_pip//:requirements.bzl", "requirement")
load(
    "@io_bazel_rules_python//python:python.bzl",
    "py_binary",
    "py_library",
    "py_test",
)
load("@subpar//:subpar.bzl", "par_binary")

filegroup(
    name = "fuzzer_configs",
    srcs = glob(["config/fuzzer/*.yaml"]),
    visibility = ["//:__subpackages__"],
)

filegroup(
    name = "testdata",
    srcs = [
        "test/build.sh",
        "test/hello.c",
        "test/mock-cc",
        "test/mock_cc_config.yaml",
        "test/test_fuzzer_config.yaml",
    ],
    visibility = ["//:__subpackages__"],
)

par_binary(
    name = "intercept",
    srcs = glob(["*.py"]),
    data = [
        ":fuzzer_configs",
        "//build_system/preload_interceptor:preload_interceptor.so",
    ],
    main = "main.py",
    visibility = ["//visibility:public"],
    zip_safe = False,
    deps = [
        "//build_system/proto:py",
        requirement("grpcio"),
        requirement("protobuf"),
        requirement("pyyaml"),
    ],
)

py_test(
    name = "grpc_test",
    size = "small",
    srcs = ["test/grpc_test.py"],
    data = [":testdata"],
    deps = [":intercept"],
)

py_test(
    name = "compilation_db_test",
    size = "small",
    srcs = ["test/compilation_db_test.py"],
    data = [":testdata"],
    deps = [":intercept"],
)

py_test(
    name = "split_argv_test",
    size = "small",
    srcs = ["test/split_argv_test.py"],
    deps = [":intercept"],
)

py_test(
    name = "integration_test",
    size = "small",
    srcs = ["test/integration_test.py"],
    data = [":testdata"],
    deps = [":intercept"],
)

py_test(
    name = "acceptance_test",
    size = "small",
    srcs = ["test/acceptance_test.py"],
    data = [":testdata"],
    deps = [":intercept"],
)

py_test(
    name = "intercept_settings_test",
    size = "small",
    srcs = ["test/intercept_settings_test.py"],
    data = [":testdata"],
    deps = [":intercept"],
)

sh_test(
    name = "lint",
    size = "small",
    srcs = ["test/lint.sh"],
    data = glob(["**/*.py"]),
)
