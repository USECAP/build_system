// Copyright (c) 2018 University of Bonn.

#pragma once

#include "absl/types/optional.h"
#include "build_system/intercept_support/compilation_command.h"
#include "build_system/proto/intercept.pb.h"

class Replacer {
 public:
  Replacer(const InterceptSettings &settings) : settings_(settings){};

  absl::optional<CompilationCommand> Replace(
      CompilationCommand original_cc) const;

 private:
  void AddArguments(CompilationCommand::ArgsT *arguments,
                    const MatchingRule &rule) const;

  void RemoveArguments(CompilationCommand::ArgsT *arguments,
                       const MatchingRule &rule) const;

  absl::optional<MatchingRule> GetMatchingRule(std::string command_path) const;

  const InterceptSettings &settings_;
};
