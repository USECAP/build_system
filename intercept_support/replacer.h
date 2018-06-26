// Copyright (c) 2018 University of Bonn.

#pragma once

#include "absl/types/optional.h"
#include "build_system/intercept_support/matcher.h"
#include "build_system/intercept_support/compilation_command.h"
#include "build_system/proto/intercept.pb.h"

class Replacer {
 public:
  Replacer(const InterceptSettings &settings)
      : settings_(settings), matcher_(settings){};

  absl::optional<CompilationCommand> Replace(
      const CompilationCommand &original_cc) const;

 private:
  std::string AdjustCommand(const std::string &command) const;

  void AddArguments(const CompilationCommand &original_command,
                    CompilationCommand::ArgsT *arguments) const;

  bool RemoveArguments(const CompilationCommand &original_cc,
                       CompilationCommand::ArgsT *arguments) const;

  const InterceptSettings &settings_;
  Matcher matcher_;
};
