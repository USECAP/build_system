// Copyright (c) 2018 University of Bonn.

#include "build_system/intercept_support/replacer.h"
#include "build_system/intercept_support/cc_arg_info.h"
#include "build_system/intercept_support/filesystem.h"
#include "re2/re2.h"

absl::optional<CompilationCommand> Replacer::Replace(
    CompilationCommand cc) const {
  // Check if we match the compilation command
  auto matching_rule = matcher_.GetMatchingRule(cc.command);
  if (!matching_rule) return {};

  RemoveArguments(cc.command, &cc.arguments);
  AddArguments(cc.command, &cc.arguments);

  cc.command = AdjustCommand(std::move(cc.command));
  // change argument zero (aka the program name)
  cc.arguments.front() = cc.command;
  return cc;
}

std::string Replacer::AdjustCommand(std::string command_path) const {
  auto command = fs::path(std::move(command_path)).filename().string();

  for (const auto &setting : settings_.matching_rules()) {
    if (!RE2::FullMatch(command, setting.match_command())) continue;

    // Don't replace with an empty string
    if (setting.replace_command().empty()) continue;

    return setting.replace_command();
  }

  return command;
}

void Replacer::AddArguments(const std::string &original_command,
                            CompilationCommand::ArgsT *arguments) const {
  auto rule = matcher_.GetMatchingRule(original_command);
  if (!rule) return;

  for (const auto &argument : rule->add_arguments()) {
    arguments->emplace_back(std::move(argument));
  }
}

namespace {
template <typename Iter>
auto getArity(Iter option, Iter end) {
  int arity = 0;

  auto arity_it = CC_ARGUMENTS_INFO.find(*option);
  if (arity_it != CC_ARGUMENTS_INFO.end()) return arity_it->second.arity;

  // fallback
  auto opt_argument = std::next(option);
  while (opt_argument != end && opt_argument->front() != '-') ++arity;

  return arity;
}

}  // anonymous namespace

void Replacer::RemoveArguments(const std::string &original_cc,
                               CompilationCommand::ArgsT *arguments) const {
  auto rule = matcher_.GetMatchingRule(original_cc);
  if (!rule) return;

  auto &remove_args = rule->remove_arguments();
  for (auto arg = remove_args.begin(); arg != remove_args.end(); ++arg) {
    auto arity = getArity(arg, remove_args.end());
    auto del_it = std::find(arguments->begin(), arguments->end(), *arg);
    for (int i = 0; i < arity + 1 && del_it != arguments->end(); i++) {
      del_it = arguments->erase(del_it);
    }
  }
}
