// Copyright (c) 2018 Code Intelligence. All rights reserved.

#include "build_system/intercept_support/compilation_command.h"
#include <iostream>

::std::ostream& operator<<(::std::ostream& os, const CompilationCommand& cc) {
  return os << cc.command;
}
