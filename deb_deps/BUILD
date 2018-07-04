cc_binary(
    name = "deb_deps",
    srcs = glob([
        "*.cc",
        "*.h",
    ]),
    data = [":packages"],
    visibility = ["//visibility:public"],
    deps = [
        "@abseil//absl/strings",
        "@abseil//absl/types:optional",
        "@bazel_tools//tools/cpp/runfiles",
        "@google_re2//:re2",
    ],
)

py_test(
    name = "deb_deps_test",
    size = "small",
    srcs = ["deb_deps_test.py"],
    data = [
        ":deb_deps",
        ":test_package",
    ],
)

genrule(
    name = "packages",
    outs = [
        "packages/xenial",
        "packages/artful",
    ],
    cmd = "./$(location fetch_package_contents.sh) $(OUTS)",
    tools = ["fetch_package_contents.sh"],
)

genrule(
    name = "test_package",
    srcs = ["packages/xenial"],
    outs = ["packages/test"],
    cmd = "cat $< | grep libmp3lame > $@",
)
