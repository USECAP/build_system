// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include <dlfcn.h>
#include <iostream>
#include "intercept_settings.h"

extern "C" {

int execve(const char *path, char *const argv[], char *const envp[]) {
  FetchSettings();
  auto orig_execve =
      reinterpret_cast<decltype(&execve)>(dlsym(RTLD_NEXT, "execve"));
  return (*orig_execve)(path, argv, {NULL});
}
}
