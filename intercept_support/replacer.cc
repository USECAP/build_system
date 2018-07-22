// Copyright (c) 2018 University of Bonn.

#include "build_system/intercept_support/replacer.h"
#include "build_system/intercept_support/cc_arg_info.h"
#include "build_system/utility/filesystem.h"
#include "build_system/utility/path.h"
#include "re2/re2.h"

namespace {

auto getArity(absl::string_view argument) {
  int arity = 0;

  auto arity_it = CC_ARGUMENTS_INFO.find(argument);
  if (arity_it != CC_ARGUMENTS_INFO.end()) return arity_it->second.arity;

  return arity;
}

}  // anonymous namespace

absl::optional<CompilationCommand> Replacer::Replace(
    CompilationCommand cc) const {
  auto rule = GetMatchingRule(cc.command);
  if (!rule) return {};

  if (rule->replace_command().empty()) return cc;

  RemoveArguments(&cc.arguments, *rule);
  AddArguments(&cc.arguments, *rule);

  cc.command = rule->replace_command();

  auto command_path = get_absolute_command_path(std::move(cc.command));
  if (!command_path) {
    std::cerr << "No absolute path could be built.\n";
    return {};
  }

  cc.command = *command_path;
  cc.arguments.front() = basename(cc.command);
  return cc;
}

void Replacer::AddArguments(CompilationCommand::ArgsT *arguments,
                            const MatchingRule &rule) const {
  for (const auto &argument : rule.add_arguments()) {
    arguments->emplace_back(argument);
  }
}

void Replacer::RemoveArguments(CompilationCommand::ArgsT *arguments,
                               const MatchingRule &rule) const {
  for (const auto &removable_arg : rule.remove_arguments()) {
    auto arity = getArity(removable_arg);
    auto del_it =
        std::find(arguments->begin(), arguments->end(), removable_arg);
    for (int i = 0; i < arity + 1 && del_it != arguments->end(); i++) {
      del_it = arguments->erase(del_it);
    }
  }
}

absl::optional<MatchingRule> Replacer::GetMatchingRule(
    std::string command_path) const {
  auto command = fs::path(std::move(command_path)).filename().string();

  for (auto rule : settings_.matching_rules()) {
    if (RE2::FullMatch(command, rule.match_command())) {
      return rule;
    }
  }
  return {};
}
