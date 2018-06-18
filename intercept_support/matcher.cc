// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include "build_system/intercept_support/matcher.h"
#include "re2/re2.h"

namespace {
struct SharedLibraryPattern {
  std::string cc_pattern;
  std::vector<std::string> shared_lib_flags;
};
const SharedLibraryPattern SHARED_LIB_PATTERNS[] = {
    {"^([^-]*-)*[mg]cc(-\\d+(\\.\\d+){0,2})?$", {"-s", "-shared", "-rdynamic"}},
    {"^([^-]*-)*clang(-\\d+(\\.\\d+){0,2})?$", {"-shared"}}};
}  // namespace

absl::optional<MatchingRule> Matcher::GetMatchingRule(
    const std::string &command) const {
  for (auto rule : settings_.matching_rules()) {
    if (RE2::FullMatch(command, rule.match_command())) {
      return rule;
    }
  }
  return {};
}

bool Matcher::doesCompileSharedLib(const CompilationCommand &cc) const {
  for (const auto &pattern : SHARED_LIB_PATTERNS) {
    if (RE2::FullMatch(cc.command, pattern.cc_pattern)) {
      for (const auto &flag : pattern.shared_lib_flags) {
        if (std::find(cc.arguments.begin(), cc.arguments.end(), flag) !=
            cc.arguments.end()) {
          return true;
        }
      }
    }
  }
  return false;
}
