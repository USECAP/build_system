// Copyright (c) 2018 Code Intelligence. All rights reserved.
#pragma once

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

#include "build_system/intercept_support/types.h"
#include "build_system/proto/intercept.pb.h"

class Matcher {
 public:
  Matcher(const InterceptSettings& settings) : settings_(settings){};

  absl::optional<MatchingRule> GetMatchingRule(
      const std::string& command) const;
  bool doesCompileSharedLib(const CompilationCommand& cc) const;

 private:
  const InterceptSettings& settings_;
};