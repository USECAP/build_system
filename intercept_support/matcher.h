// Copyright (c) 2018 University of Bonn.
#pragma once

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

#include "build_system/intercept_support/compilation_command.h"
#include "build_system/proto/intercept.pb.h"

class Matcher {
 public:
  Matcher(const InterceptSettings& settings) : settings_(settings){};

  absl::optional<MatchingRule> GetMatchingRule(std::string command_path) const;
  bool doesCompileSharedLib(const CompilationCommand& cc) const;

 private:
  const InterceptSettings& settings_;
};