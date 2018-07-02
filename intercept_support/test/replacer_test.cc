// Copyright (c) 2018 University of Bonn.

#include "build_system/intercept_support/replacer.h"
#include "gtest/gtest.h"

namespace {
const auto C_PATTERN =
    "^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?$|"
    "^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?$|"
    "^(|i)cc$|^(g|)xlc$";

struct ReplacerTestCase {
  CompilationCommand cc;
  CompilationCommand::ArgsT add_arguments;
  CompilationCommand::ArgsT remove_arguments;
  CompilationCommand::ArgsT expected_args;
};
}  // namespace

const ReplacerTestCase ReplacerTestCases[] = {
    ReplacerTestCase{
        CompilationCommand{"/usr/bin/gcc", {"gcc", "hello.c", "-o", "foo"}},
        {"-O0"},
        {"-O3"},
        {"afl-gcc", "hello.c", "-o", "foo", "-O0"}},
    ReplacerTestCase{
        CompilationCommand{
            "clang", {"clang", "test.cpp", "-I", "DIR", "-O2", "-o", "test.o"}},
        {"-O0"},
        {"-O2", "-I"},
        {"afl-gcc", "test.cpp", "-o", "test.o", "-O0"}}};

TEST(Replacer, Replace) {
  for (auto tc : ReplacerTestCases) {
    InterceptSettings settings;
    auto rule = settings.add_matching_rules();
    rule->set_match_command(C_PATTERN);
    rule->set_replace_command("afl-gcc");
    *rule->mutable_add_arguments() = {tc.add_arguments.begin(),
                                      tc.add_arguments.end()};
    *rule->mutable_remove_arguments() = {tc.remove_arguments.begin(),
                                         tc.remove_arguments.end()};

    auto result = Replacer{settings}.Replace(tc.cc);

    EXPECT_EQ(result->command, "afl-gcc");
    EXPECT_EQ(result->arguments, tc.expected_args);
  }
}

TEST(Replacer, EmptyReplacement_ShouldKeepOriginalCommand) {
  InterceptSettings settings;
  auto rule = settings.add_matching_rules();
  rule->set_match_command(C_PATTERN);
  rule->set_replace_command("");

  CompilationCommand cc("gcc", {"gcc", "test.c", "-o", "test"});
  auto replaced_cc = Replacer(settings).Replace(cc);
  ASSERT_EQ(replaced_cc->command, "gcc");
}
