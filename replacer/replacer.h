// Copyright (c) 2018 University of Bonn.

#pragma once

#include "absl/types/optional.h"
#include "build_system/replacer/compilation_command.h"
#include "build_system/proto/intercept.pb.h"

class Replacer {
 public:
  explicit Replacer(const InterceptSettings &settings) : settings_(settings){};

  /// Transforms original_cc according to a rule in settings if matched by that
  /// rule. The new CompilationCommand contains an absolute command path.
  /// @returns CompilationCommand if succeeds, nothing if no rule could be
  /// matched or no absolute path could be found
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
