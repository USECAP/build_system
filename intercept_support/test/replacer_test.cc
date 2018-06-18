// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include "gtest/gtest.h"
#include "build_system/intercept_support/replacer.h"

TEST(Replacer, Replace) {
  CompilationCommand orig_cc {"gcc", {"test.cpp", "-o", "test.o"}};

  InterceptSettings settings;
  auto* rule = settings.add_matching_rules();
  rule->set_match_command("^gcc");
  rule->set_replace_command("afl-gcc");
  rule->add_add_arguments("-g");
  rule->add_add_arguments("-O0");

  Replacer r(settings);

  CompilationCommand adjusted_cc = r.Replace(orig_cc);
  ASSERT_EQ(adjusted_cc.command, "afl-gcc");
  ASSERT_EQ(adjusted_cc.arguments.size(), 5);
  ASSERT_EQ(adjusted_cc.arguments[0], "test.cpp");
  ASSERT_EQ(adjusted_cc.arguments[1], "-o");
  ASSERT_EQ(adjusted_cc.arguments[2], "test.o");
  ASSERT_EQ(adjusted_cc.arguments[3], "-g");
  ASSERT_EQ(adjusted_cc.arguments[4], "-O0");
}