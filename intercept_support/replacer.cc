// Copyright (c) 2018 University of Bonn.

#include "build_system/intercept_support/replacer.h"
#include "build_system/intercept_support/cc_arg_info.h"

#include "re2/re2.h"

CompilationCommand Replacer::Replace(
  const CompilationCommand &original_cc) const {
  CompilationCommand adjusted_cc;
  adjusted_cc.command = AdjustCommand(original_cc.command);
  adjusted_cc.arguments.insert(adjusted_cc.arguments.end(),
                               original_cc.arguments.begin(),
                               original_cc.arguments.end());
  RemoveArguments(original_cc, &adjusted_cc.arguments);
  AddArguments(original_cc, &adjusted_cc.arguments);
  return adjusted_cc;
}

std::string Replacer::AdjustCommand(const std::string &command) const {
  for (const auto &setting : settings_.matching_rules()) {
    if (RE2::FullMatch(command, setting.match_command())) {
      return setting.replace_command();
    }
  }
  return command;
}

void Replacer::AddArguments(const CompilationCommand &original_command,
                            std::vector<std::string> *arguments) const {
  if (auto rule = matcher_.GetMatchingRule(original_command.command)) {
    for (const auto &argument : rule->add_arguments()) {
      arguments->push_back(argument);
    }
  }
}

bool Replacer::RemoveArguments(const CompilationCommand &original_cc, std::vector<std::string> *arguments) const {
  if (auto rule = matcher_.GetMatchingRule(original_cc.command)) {
    for (const auto &arg : rule->remove_arguments()) {
      auto arg_it = CC_ARGUMENTS_INFO.find(arg);
      if (arg_it == CC_ARGUMENTS_INFO.end()) {
        return false;
      }
      auto del_it = std::find(arguments->begin(), arguments->end(), arg);
      arguments->erase(del_it, del_it + arg_it->second.arity + 1);
    }
    return true;
  }
  return false;
}
