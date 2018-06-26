#include "build_system/intercept_support/matcher.h"
#include "gtest/gtest.h"

namespace {
const auto C_PATTERN =
    "^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?$|"
    "^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?$|"
    "^(|i)cc$|^(g|)xlc$";

struct SharedLibTestCase {
  CompilationCommand cc;
  bool is_shared;
};
}  // namespace

class SharedLibraryTest : public testing::TestWithParam<SharedLibTestCase> {
 public:
  virtual void SetUp() {
    InterceptSettings settings;
    matcher = new Matcher(settings);
  };
  virtual void TearDown() { delete (matcher); };

  Matcher *matcher;
};

INSTANTIATE_TEST_CASE_P(
    SharedLib, SharedLibraryTest,
    testing::Values(
        SharedLibTestCase{
            CompilationCommand{"gcc", {"-shared", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{
            CompilationCommand{"gcc-7.0",
                               {"-shared", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{
            CompilationCommand{
                "gcc", {"-fooflag", "-shared", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{
            CompilationCommand{"gcc",
                               {"-fooflag", "-s", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{
            CompilationCommand{
                "gcc", {"-fooflag", "-rdynamic", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{CompilationCommand{"arm-eabi-gcc",
                                             {"-fooflag", "-shared", "-o",
                                              "libfoo.so", "foo.o"}},
                          true},
        SharedLibTestCase{CompilationCommand{
                              "clang", {"-shared", "-o", "libfoo.so", "foo.o"}},
                          true},
        SharedLibTestCase{
            CompilationCommand{"clang-5.0",
                               {"-shared", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{
            CompilationCommand{
                "clang", {"-fooflag", "-shared", "-o", "libfoo.so", "foo.o"}},
            true},
        SharedLibTestCase{CompilationCommand{"gcc", {"-o", "foo", "foo.o"}},
                          false},
        SharedLibTestCase{CompilationCommand{"clang", {"-o", "foo", "foo.o"}},
                          false},
        SharedLibTestCase{CompilationCommand{"foocc", {"-o", "foo", "foo.o"}},
                          false}));

TEST_P(SharedLibraryTest, IsShared) {
  ASSERT_EQ(matcher->doesCompileSharedLib(GetParam().cc), GetParam().is_shared);
}

struct MatchingCommandTestCase {
  std::string command_pattern;
  CompilationCommand cc;
  bool does_match;
};

class MatchingCommandTest
    : public testing::TestWithParam<MatchingCommandTestCase> {
 public:
  virtual void SetUp() { matcher = new Matcher(settings); }

  virtual void TearDown() { delete (matcher); }

  InterceptSettings settings;
  Matcher *matcher;
};

INSTANTIATE_TEST_CASE_P(
    Match, MatchingCommandTest,
    testing::Values(
        MatchingCommandTestCase{
            C_PATTERN,
            CompilationCommand{"/usr/bin/gcc",
                               {"-o", "libfoo.so", "foo", ".o"}},
            true},
        MatchingCommandTestCase{
            C_PATTERN,
            CompilationCommand{"clang", {"-o", "libfoo.so", "foo", ".o"}},
            true},
        MatchingCommandTestCase{
            C_PATTERN,
            CompilationCommand{"foo-compiler", {"-o", "libfoo.so", "foo.o"}},
            false}));

TEST_P(MatchingCommandTest, GetMatch_Match) {
  auto *rule = settings.add_matching_rules();
  rule->set_match_command(GetParam().command_pattern);
  ASSERT_EQ(matcher->GetMatchingRule(GetParam().cc.command).has_value(),
            GetParam().does_match);
}
