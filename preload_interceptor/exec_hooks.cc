// Copyright (c) 2018 University of Bonn.

#include <dlfcn.h>
#include <fcntl.h>
#include <google/protobuf/text_format.h>
#include <unistd.h>
#include <cstdarg>
#include <iostream>
#include <vector>
#include "absl/types/optional.h"
#include "build_system/replacer/replacer.h"
#include "intercept_settings.h"
#include "build_system/replacer/path.h"

namespace {

void unhook() { unsetenv("LD_PRELOAD"); }

void unhook(char *const **envp) {
  environ = const_cast<char **>(*envp);
  unhook();
  envp = const_cast<char *const **>(&environ);
}

/// converts a va_list of char* to a vector
std::vector<char *> list_to_vector(va_list &args, const char *first_arg) {
  std::vector<char *> result;
  auto next_arg = (char *)first_arg;
  while (!next_arg) {
    result.emplace_back(next_arg);
    next_arg = va_arg(args, char *);
  }
  return result;
}

/// prepares the CompilationCommand arguments so they can be passed to exec
std::vector<char *> to_argv(const CompilationCommand &cc) {
  std::vector<char *> arguments;
  arguments.reserve(16);

  for (auto &arg : cc.arguments) {
    if (!arg.empty()) arguments.emplace_back(const_cast<char *>(arg.data()));
  }

  if (arguments.back() != nullptr) arguments.emplace_back(nullptr);

  return arguments;
}

/// replaces cmd according to the configuration if possible
absl::optional<CompilationCommand> replace_compilation_command(
    CompilationCommand cmd) {
  InterceptSettings settings;
  std::string settings_env{std::getenv("INTERCEPT_SETTINGS")};
  if (!google::protobuf::TextFormat::ParseFromString(settings_env, &settings)) {
    std::cerr << "Settings could not be read!\n";
    std::cerr << "Tried to parse settings from environment: "
              << std::getenv("INTERCEPT_SETTINGS") << "\n";
    return {};
  }

  Replacer replacer(settings);
  return replacer.Replace(std::move(cmd));
}

/// reports the original and replaced compilation commands to the grpc server
/// if possible
void report_replacement(const CompilationCommand &original,
                        const CompilationCommand &replaced) {
  auto reportUrl = std::getenv("REPORT_URL");
  if (reportUrl != nullptr) {
    InterceptorClient client(
        grpc::CreateChannel(reportUrl, grpc::InsecureChannelCredentials()));
    client.ReportInterceptedCommand(original, replaced);
  }
}

template <typename... Args>
int intercept(const char *fn_name, const char *path, char *const argv[],
              Args... envp) {
  using exec_type = int (*)(const char *, char *const *, Args...);
  auto original_exec = reinterpret_cast<exec_type>(dlsym(RTLD_NEXT, fn_name));

  CompilationCommand command(path, argv);

  auto replaced_command = replace_compilation_command(command);
  if (!replaced_command) {
    return original_exec(path, argv, envp...);
  }

  report_replacement(command, *replaced_command);
  unhook(&envp...);
  auto exec_arguments = to_argv(*replaced_command);

  replaced_command->command = get_absolute_command_path(replaced_command->command);

  return original_exec(replaced_command->command.data(), exec_arguments.data(),
                       envp...);
}

}  // namespace

extern "C" {

// Hook these methods
int execve(const char *path, char *const argv[], char *const envp[]) {
  return intercept("execve", path, argv, envp);
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
  return intercept("execvpe", file, argv, envp);
}

int execv(const char *path, char *const argv[]) {
  return intercept("execv", path, argv);
}

int execvp(const char *file, char *const argv[]) {
  return intercept("execvp", file, argv);
}

// Convert from variadic arguments here and delegate to hooked methods.
int execl(const char *path, const char *first_arg, ...) {
  va_list args;
  va_start(args, first_arg);
  auto arguments = list_to_vector(args, first_arg);
  va_end(args);

  return execv(path, const_cast<char *const *>(arguments.data()));
}

int execle(const char *path, const char *first_arg, ...) {
  va_list args;
  va_start(args, first_arg);
  auto arguments = list_to_vector(args, first_arg);
  auto envp = va_arg(args, char *const *);
  va_end(args);

  return execve(path, const_cast<char *const *>(arguments.data()), envp);
}

int execlp(const char *file, const char *first_arg, ...) {
  va_list args;
  va_start(args, first_arg);
  auto arguments = list_to_vector(args, first_arg);
  va_end(args);

  return execvp(file, const_cast<char *const *>(arguments.data()));
}

}  // extern C
