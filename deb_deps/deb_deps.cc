
#include <cassert>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "re2/re2.h"
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

namespace {
absl::optional<std::ifstream> open_runfile(const Runfiles &runfiles,
                                           const std::string &path);
void process_stream(absl::string_view library_name, std::ifstream &ifstream);

constexpr auto PACKAGES_BASE =
    "code_intelligence/build_system/deb_deps/packages/";

void process_stream(absl::string_view library_name, std::ifstream &ifstream) {
  std::string line;
  std::stringstream regex_ss;
  std::string full_package_path;
  std::set<std::string> dependencies;

  regex_ss << library_name << "[\\d\\.]*\\s+[\\w/]+/([^/]+)$";
  RE2 regex{regex_ss.str()};

  while (std::getline(ifstream, line)) {
    if (RE2::PartialMatch(line, regex, &full_package_path)) {
      dependencies.insert(full_package_path);
    }
  }

  // print results
  for (auto &dep : dependencies) {
    std::cout << dep << " ";
  }
  std::cout << std::endl;
}

absl::optional<std::ifstream> open_runfile(const Runfiles &runfiles,
                                           const std::string &path) {
  std::string location = runfiles.Rlocation(path);
  if (path.empty()) {
    std::cerr << "Error: Runfile not found!" << std::endl;
    return {};
  }
  auto file_stream = absl::make_optional<std::ifstream>(location.c_str());
  if (!file_stream->good()) {
    std::cerr << "Error opening " << path << ": Runfile not found!"
              << std::endl;
    return {};
  }
  return file_stream;
}
}  // namespace

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "USAGE: " << argv[0] << " distro library" << std::endl;
    return -1;
  }
  std::string distro{argv[1]};
  std::string library_name{argv[2]};
  std::string error;
  std::unique_ptr<Runfiles> runfiles{Runfiles::Create(argv[0], &error)};
  if (!runfiles) {
    std::cerr << "Error getting runfiles: " << error << std::endl;
    return -1;
  }
  auto input = open_runfile(*runfiles, PACKAGES_BASE + distro);
  if (input) {
    process_stream(library_name, *input);
  }

  return 0;
}
