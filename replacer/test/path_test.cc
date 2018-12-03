#include "build_system/replacer/path.h"
#include "gtest/gtest.h"
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <ftw.h>

struct PathResolverTest : public ::testing::Test {
  std::string false_root;
  std::string path_env;

  PathResolverTest() : false_root(current_directory()), path_env{"PATH="} {
    // set environment
    auto false_path = false_root + "/sandbox/bin";
    path_env += false_path;
    putenv(const_cast<char *>(path_env.data()));
    const int mode = 0700;

    mkdir("sandbox", mode);
    mkdir("sandbox/bin", mode);
    mkdir("sandbox/home", mode);
    mkdir("sandbox/home/dir1", mode);
    mkdir("sandbox/home/dir1/dir2", mode);
    touch("sandbox/bin/true");
    touch("sandbox/home/dir1/true");
  }

  ~PathResolverTest() override {
    chdir(false_root.data());
    char sandbox[] = "sandbox";
    rmrf(sandbox);
  }
};

TEST_F(PathResolverTest, does_resolve_filenames) {
  chdir("sandbox/home/dir1/dir2");
  auto path = get_absolute_command_path("true");
  auto expected_Path = false_root + "/sandbox/bin/true";
  EXPECT_EQ(path, expected_Path);
}
