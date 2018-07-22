#include "path.h"
#include <iostream>
#include "absl/strings/str_split.h"
#include "filesystem.h"
#include "re2/re2.h"

namespace {

bool is_not_a_path(absl::string_view s) {
  return RE2::FullMatch(s.data(), R"([a-zA-Z0-9._+-]+)");
}

}  // namespace

class FileFinder {
  std::string file_name;
  std::vector<std::string> paths;
  std::vector<std::string>::const_iterator current_pos = paths.cbegin();

 public:
  explicit FileFinder(std::string file_name);

  absl::optional<std::string> get_candidate();

 private:
  const char *get_path_env() const;

  decltype(paths) get_paths() const;
};

absl::optional<std::string> FileFinder::get_candidate() {
  while (current_pos != paths.end()) {
    auto path = fs::path(*current_pos) / file_name;
    ++current_pos;
    if (!fs::exists(path)) continue;

    try {
      auto canonical_path = fs::canonical(path);
      return canonical_path.string();
    } catch (fs::filesystem_error &e) {
      continue;
    }
  }

  return {};
}

FileFinder::FileFinder(std::string file_name)
    : file_name{std::move(file_name)}, paths{get_paths()} {}

auto FileFinder::get_paths() const -> decltype(paths) {
  return absl::StrSplit(get_path_env(), ':');
}

const char *FileFinder::get_path_env() const {
  auto path_env = std::getenv("PATH");
  if (!path_env) return "/bin:/usr/bin";
  return path_env;
}

absl::optional<std::string> get_absolute_command_path(std::string command) {
  if (is_not_a_path(command)) {
    auto ff = FileFinder(std::move(command));
    return ff.get_candidate();
  }

  try {
    auto path = fs::canonical(fs::path(command.data()));
    return path.string();
  } catch (fs::filesystem_error &e) {
    return {};
  }
}

std::string basename(absl::string_view s) {
  return fs::path(s.data()).filename().string();
}
