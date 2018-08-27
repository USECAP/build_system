#pragma once

#include <vector>
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

absl::optional<std::string> get_absolute_command_path(std::string command);

std::string basename(absl::string_view s);

std::string current_directory();

int rmrf(char *path);

int touch(const char *name);

