// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include "build_system/intercept_support/matcher.h"
#include "re2/re2.h"

absl::optional<MatchingRule> Matcher::GetMatchingRule(const std::string &command) {
  for (auto rule : settings_.matching_rules()) {
    if (RE2::FullMatch(command, rule.match_command())) {
      return rule;
    }
  }
  return {};
}

std::string Matcher::GetParameters(const std::string &command, const MatchingRule &matching_rule) {
  std::string params;
  RE2::FullMatch(command, matching_rule.match_command(), &params);
  return std::move(params);
}