// Copyright (c) 2018 University of Bonn.

#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdarg>
#include <iostream>
#include <vector>
#include "absl/types/optional.h"
#include "build_system/intercept_support/replacer.h"
#include "intercept_settings.h"

namespace {

std::vector<char *> list_to_vector(va_list &args, const char *first_arg) {
  std::vector<char *> result;
  auto next_arg = (char *)first_arg;
  while (!next_arg) {
    result.emplace_back(next_arg);
    next_arg = va_arg(args, char *);
  }
  return result;
}

std::vector<char *> prepare_arguments(const CompilationCommand &cc) {
  std::vector<char *> arguments;
  arguments.reserve(16);

  for (auto &arg : cc.arguments) {
    if (!arg.empty()) arguments.emplace_back(const_cast<char *>(arg.data()));
  }

  if (arguments.back() != nullptr) arguments.emplace_back(nullptr);

  return arguments;
}

absl::optional<InterceptSettings> settings_;

InterceptSettings *settings() {
  if (!settings_) {
    auto reportUrl = std::getenv("REPORT_URL");
    if (reportUrl == nullptr) {
      std::cerr << "Error fetching settings: REPORT_URL not set!" << std::endl;
      settings_ = InterceptSettings();
      return &*settings_;
    }

    InterceptorClient client(grpc::CreateChannel(
        std::getenv("REPORT_URL"), grpc::InsecureChannelCredentials()));
    auto maybe_settings = client.GetSettings();
    if (!maybe_settings) {
      std::cerr << "Error fetching settings!" << std::endl;
      settings_ = InterceptSettings();
    } else {
      settings_ = *maybe_settings;
    }
  }
  return &*settings_;
}

void unset_ld_preload() { unsetenv("LD_PRELOAD"); }

void unset_ld_preload(char *const **envp) {
  // TODO Clear environment from parameter
}

template <typename... Args>
int intercept(const char *fn_name, const char *path, char *const argv[],
              Args... envp) {
  using exec_type = int (*)(const char *, char *const *, Args...);
  auto call = reinterpret_cast<exec_type>(dlsym(RTLD_NEXT, fn_name));

  auto settings_ = settings();
  if (!settings_) {
    std::cerr << "Settings could not be fetched!\n";
    return call(path, argv, envp...);
  }

  Replacer replacer(*settings_);
  CompilationCommand cc(path, argv);
  auto replaced_cc = replacer.Replace(cc);

  if (!replaced_cc) return call(path, argv, envp...);

  unset_ld_preload(&envp...);

  auto reportUrl = std::getenv("REPORT_URL");
  if (reportUrl != nullptr) {
    InterceptorClient client(
        grpc::CreateChannel(reportUrl, grpc::InsecureChannelCredentials()));
    client.ReportInterceptedCommand(cc, *replaced_cc);
  }

  auto exec_arguments = prepare_arguments(*replaced_cc);
  return call(replaced_cc->command.data(), exec_arguments.data(), envp...);
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
