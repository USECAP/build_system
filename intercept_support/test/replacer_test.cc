// Copyright (c) 2018 University of Bonn.

#include "gtest/gtest.h"
#include "build_system/intercept_support/replacer.h"

struct ReplacerTestCase {
  CompilationCommand cc;
  std::vector<std::string> add_arguments;
  std::vector<std::string> remove_arguments;
  std::vector<std::string> result;
};

class ReplacerTest : public testing::TestWithParam<ReplacerTestCase> {
 public:
  virtual void SetUp() {
    replacer = new Replacer(settings);
  }

  virtual void TearDown() {
    delete (replacer);
  }

  InterceptSettings settings;
  Replacer *replacer;
};

INSTANTIATE_TEST_CASE_P(Test, ReplacerTest, testing::Values(
  ReplacerTestCase{{"gcc", {"test.cpp", "-O3", "-o", "test.o"}},
                   {"-O0"}, {"-O3"}, {"test.cpp", "-o", "test.o", "-O0"}},
  ReplacerTestCase{{"gcc", {"test.cpp", "-I", "DIR", "-O2", "-o", "test.o"}},
                   {"-O0"}, {"-O2", "-I"}, {"test.cpp", "-o", "test.o", "-O0"}}
));

TEST_P(ReplacerTest, GCC) {
  auto *rule = settings.add_matching_rules();
  rule->set_match_command("gcc");
  rule->set_replace_command("afl-gcc");
  for (const auto &arg : GetParam().add_arguments) {
    rule->add_add_arguments(arg);
  }
  for (const auto &arg : GetParam().remove_arguments) {
    rule->add_remove_arguments(arg);
  }

  CompilationCommand adjusted_cc = replacer->Replace(GetParam().cc);
  ASSERT_EQ(adjusted_cc.command, rule->replace_command());
  ASSERT_EQ(adjusted_cc.arguments.size(), GetParam().result.size());
  ASSERT_TRUE(std::equal(adjusted_cc.arguments.begin(), adjusted_cc.arguments.end(), GetParam().result.begin()));
}

