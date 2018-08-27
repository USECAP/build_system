#include "path.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ftw.h>

#include <iostream>

#include "absl/strings/str_split.h"
#include "re2/re2.h"

namespace {

bool is_not_a_path(absl::string_view s) {
  return RE2::FullMatch(s.data(), R"([a-zA-Z0-9._+-]+)");
}

absl::optional<std::string> make_canonical_path(std::string path) {
  char *canonical_path = realpath(path.data(), NULL);
  if (!canonical_path) return {};

  auto result = std::string(canonical_path);
  free(canonical_path);
  return result;
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
    auto path = *current_pos + "/" + file_name;
    ++current_pos;
    if (access(path.data(), F_OK) != 0) continue;

    auto canonical_path = make_canonical_path(std::move(path));
    if (!canonical_path) continue;

    return *canonical_path;
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

  return make_canonical_path(std::move(command));
}

std::string basename(absl::string_view s) {
  auto basename_start = std::find(s.rbegin(), s.rend(), '/');
  if (basename_start == s.rend()) return std::string(s.data());
  return std::string(basename_start.base(), s.end());
}

std::string current_directory() {
  char buffer[PATH_MAX] = {0};
  return std::string(getcwd(buffer, sizeof(buffer)));
}

namespace {
int unlink_cb(const char *fpath, const struct stat *sb,
              int typeflag, struct FTW *ftwbuf) {
  int rv = remove(fpath);
  if (rv) perror(fpath);
  return rv;
}

}

int rmrf(char *path) {
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int touch(const char *name) {
    FILE *file = fopen(name, "w");
    if (!file) return 1;
    return fclose(file);
}
