#include "build_system/intercept_support/compilation_command.h"
#include "gtest/gtest.h"

TEST(CompilationCommand, ConstructFromCArgsArray) {
  const char *argv[4] = {"foo", "bar", "baz", nullptr};
  CompilationCommand cc("cc", reinterpret_cast<const char *const *>(argv));
  ASSERT_EQ(cc.arguments, CompilationCommand::ArgsT({"foo", "bar", "baz"}));
}
