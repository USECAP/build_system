// Copyright (c) 2018 University of Bonn.

#pragma once

#include "build_system/intercept_support/matcher.h"
#include "build_system/intercept_support/types.h"
#include "build_system/proto/intercept.pb.h"

class Replacer {
 public:
  Replacer(const InterceptSettings &settings)
    : settings_(settings), matcher_(settings) {};

  CompilationCommand Replace(const CompilationCommand &original_cc) const;

 private:
  std::string AdjustCommand(const std::string &command) const;

  void AddArguments(const CompilationCommand &original_command, std::vector<std::string> *arguments) const;

  bool RemoveArguments(const CompilationCommand &original_cc, std::vector<std::string> *arguments) const;

  const InterceptSettings &settings_;
  Matcher matcher_;
};
