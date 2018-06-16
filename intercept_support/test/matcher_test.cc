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