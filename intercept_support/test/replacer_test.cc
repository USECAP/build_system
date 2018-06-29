// Copyright (c) 2018 University of Bonn.

#include "build_system/intercept_support/replacer.h"
#include "gtest/gtest.h"

namespace {
const auto C_PATTERN =
    "^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?$|"
    "^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?$|"
    "^(|i)cc$|^(g|)xlc$";

struct ReplacerTestCase {
  using ContainerT = CompilationCommand::ArgsT;
  CompilationCommand cc;
  ContainerT add_arguments;
  ContainerT remove_arguments;
  ContainerT result;
};
}  // namespace

class ReplacerTest : public testing::TestWithParam<ReplacerTestCase> {
 public:
  virtual void SetUp() { replacer = std::make_unique<Replacer>(settings); }

  InterceptSettings settings;
  std::unique_ptr<Replacer> replacer;
};

INSTANTIATE_TEST_CASE_P(
    Test, ReplacerTest,
    testing::Values(
        ReplacerTestCase{
            {"/usr/bin/gcc", {"gcc", "hello.c", "-o", "foo"}},
            {"-O0"},
            {"-O3"},
            {"afl-gcc", "hello.c", "-o", "foo", "-O0"}
        },
        ReplacerTestCase{
            {"clang",
                {"clang", "test.cpp", "-I", "DIR", "-O2", "-o", "test.o"}},
            {"-O0"},
            {"-O2", "-I"},
            {"afl-gcc", "test.cpp", "-o", "test.o", "-O0"}
        }
    )
);

TEST_P(ReplacerTest, GCC) {
  auto rule = settings.add_matching_rules();
  rule->set_match_command(C_PATTERN);
  rule->set_replace_command("afl-gcc");
  for (const auto &arg : GetParam().add_arguments) {
    rule->add_add_arguments(arg);
  }
  for (const auto &arg : GetParam().remove_arguments) {
    rule->add_remove_arguments(arg);
  }

  CompilationCommand expected_cc("afl-gcc", GetParam().result);
  auto replaced_cc = replacer->Replace(GetParam().cc);
  ASSERT_EQ(replaced_cc->command, expected_cc.command);
  ASSERT_EQ(replaced_cc->arguments, expected_cc.arguments);
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
