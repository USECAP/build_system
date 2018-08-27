// Copyright (c) 2018 University of Bonn.

#include "build_system/replacer/replacer.h"
#include "build_system/replacer/path.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "gtest/gtest.h"

namespace {
const auto C_PATTERN =
    "^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?$|"
    "^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?$|"
    "^(|i)cc$|^(g|)xlc$";

const char *REPLACE_COMPILER = "mock-cc";

struct ReplacerTestCase {
  CompilationCommand cc;
  CompilationCommand::ArgsT add_arguments;
  CompilationCommand::ArgsT remove_arguments;
  CompilationCommand::ArgsT expected_args;
};

InterceptSettings SetupSettings(const char *replace_command) {
  InterceptSettings settings;
  auto rule = settings.add_matching_rules();
  rule->set_match_command(C_PATTERN);
  rule->set_replace_command(replace_command);
  return settings;
}

void ApplyTestCase(ReplacerTestCase &tc, InterceptSettings &settings) {
  auto rule = settings.mutable_matching_rules()->begin();
  *rule->mutable_add_arguments() = {
      std::make_move_iterator(tc.add_arguments.begin()),
      std::make_move_iterator(tc.add_arguments.end())};
  *rule->mutable_remove_arguments() = {
      std::make_move_iterator(tc.remove_arguments.begin()),
      std::make_move_iterator(tc.remove_arguments.end())};
}

const ReplacerTestCase ReplacerTestCases[] = {
    ReplacerTestCase{
        CompilationCommand{"/usr/bin/gcc", {"gcc", "hello.c", "-o", "foo"}},
        {"-O0"},
        {"-O3"},
        {REPLACE_COMPILER, "hello.c", "-o", "foo", "-O0"}},
    ReplacerTestCase{
        CompilationCommand{
            "clang", {"clang", "test.cpp", "-I", "DIR", "-O2", "-o", "test.o"}},
        {"-O0"},
        {"-O2", "-I"},
        {REPLACE_COMPILER, "test.cpp", "-o", "test.o", "-O0"}}};

}  // namespace

TEST(Replacer, DoesReplaceCompilerCommand) {
  auto working_directory = current_directory();

  // setup environment
  touch(REPLACE_COMPILER);
  std::string environment = "PATH=" + working_directory;
  putenv(const_cast<char *>(environment.data()));

  for (auto tc : ReplacerTestCases) {
    InterceptSettings settings = SetupSettings(REPLACE_COMPILER);
    ApplyTestCase(tc, settings);

    auto result = Replacer{settings}.Replace(std::move(tc.cc));
    ASSERT_TRUE(result) << "Replacer could not replace the compiler command.";

    EXPECT_EQ(result->command, REPLACE_COMPILER);
    EXPECT_EQ(result->arguments, tc.expected_args);
  }

  remove((working_directory + REPLACE_COMPILER).data());
}

TEST(Replacer, EmptyReplacement_ShouldKeepOriginalCommand) {
  InterceptSettings settings = SetupSettings("");

  CompilationCommand cc("/usr/bin/gcc", {"gcc", "test.c", "-o", "test"});
  auto replaced_cc = Replacer(settings).Replace(cc);
  ASSERT_EQ(replaced_cc->command, "/usr/bin/gcc");
}
