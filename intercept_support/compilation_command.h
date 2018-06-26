// Copyright (c) 2018 University of Bonn.

#pragma once

#include <string>
#include <vector>
#include <list>

struct CompilationCommand {
  using ArgsT = std::list<std::string>;

  CompilationCommand() = default;

  CompilationCommand(std::string command, ArgsT arguments)
      : command(std::move(command)), arguments(std::move(arguments)) {}

  CompilationCommand(const char* command, const char* const argv[])
      : command(command) {
    for (int i = 0; argv[i] != nullptr; i++) {
      arguments.emplace_back(argv[i]);
    }
  }

  bool operator==(const CompilationCommand& other) const {
    return command == other.command && arguments == other.arguments;
  }

  std::string command;
  ArgsT arguments;
};
