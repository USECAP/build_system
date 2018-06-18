#include "gtest/gtest.h"
#include "build_system/intercept_support/matcher.h"

TEST(Matcher, GetMatch_Match) {
  InterceptSettings settings;
  auto* rule = settings.add_matching_rules();
  rule->set_match_command("^gcc.*");
  Matcher m(settings);
  ASSERT_NE(m.GetMatchingRule("gcc test.cpp -o test.o"), absl::nullopt);
}

TEST(Matcher, GetMatch_NoMach) {
  InterceptSettings settings;
  auto* rule = settings.add_matching_rules();
  rule->set_match_command("^gcc.*");
  Matcher m(settings);
  ASSERT_EQ(m.GetMatchingRule("clang test.cpp -o test.o"), absl::nullopt);
}

TEST(Matcher, Get_Parameters) {
  InterceptSettings settings;
  auto* rule = settings.add_matching_rules();
  rule->set_match_command("^gcc\\s(.*)");
  Matcher m(settings);
  std::string params = m.GetParameters("gcc test.cpp -o test.o", *rule);
  ASSERT_EQ(params, "test.cpp -o test.o");
}

TEST(Matcher, SharedLibraryGCC) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("gcc -shared -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryGCC_VersionNumber) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("gcc-7.0 -shared -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryGCC_FlagsInBetween) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("gcc -fooflag -shared -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryGCC_S) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("gcc -fooflag -s -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryGCC_Rdynamic) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("gcc -fooflag -rdynamic -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryGCCARM) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("arm-eabi-gcc -fooflag -shared -o libfoo.so foo.o"));
}

TEST(Matcher, NoSharedLibraryGCC) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_FALSE(m.doesCompileSharedLib("gcc -o foo foo.o"));
}

TEST(Matcher, SharedLibraryCLANG) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("clang -shared -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryClang_VersionNumber) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("clang-5.0 -shared -o libfoo.so foo.o"));
}

TEST(Matcher, SharedLibraryCLANG_FlagsInBetween) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_TRUE(m.doesCompileSharedLib("clang -fooflag -shared -o libfoo.so foo.o"));
}

TEST(Matcher, NoSharedLibraryClang) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_FALSE(m.doesCompileSharedLib("clang -o foo foo.o"));
}

TEST(Matcher, UnknownCompiler) {
  InterceptSettings settings;
  Matcher m(settings);
  ASSERT_FALSE(m.doesCompileSharedLib("foocc -o foo foo.o"));
}
