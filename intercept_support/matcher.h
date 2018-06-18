// Copyright (c) 2018 Code Intelligence. All rights reserved.
#pragma once

#include "absl/types/optional.h"
#include "absl/strings/string_view.h"

#include "build_system/proto/intercept.pb.h"

class Matcher {
 public:
  Matcher(InterceptSettings settings) : settings_(settings) {};

  absl::optional<MatchingRule> GetMatchingRule(const std::string &command);
  std::string GetParameters(const std::string &command, const MatchingRule &matching_rule);
  bool doesCompileSharedLib (const std::string& command);

 private:
  InterceptSettings settings_;
};