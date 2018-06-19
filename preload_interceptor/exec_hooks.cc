// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include <dlfcn.h>
#include <iostream>
#include "intercept_settings.h"

extern "C" {
#include <unistd.h>

int execve(const char *path, char *const argv[], char *const envp[]) {
  static InterceptorClient client = FetchSettings();
  auto orig_execve =
      reinterpret_cast<decltype(&execve)>(dlsym(RTLD_NEXT, "execve"));
  client.ReportInterceptedCommand(path, argv, path, argv);
  return (*orig_execve)(path, argv, envp);
}

int execvp(const char *file, char *const argv[]) {
  InterceptorClient client = FetchSettings();
  auto orig_execvp =
      reinterpret_cast<decltype(&execvp)>(dlsym(RTLD_NEXT, "execvp"));
  //client.get()->ReportInterceptedCommand(file, argv, file, argv);
  return (*orig_execvp)(file, argv);
}
}
