#include "build_system/utility/path.h"
#include "build_system/utility/filesystem.h"
#include "gtest/gtest.h"

struct PathResolverTest : public ::testing::Test {
  fs::path false_root;
  std::string path_env;

  PathResolverTest() : false_root(fs::current_path()), path_env{"PATH="} {
    // set environment
    auto false_path = false_root / fs::path("sandbox/bin");
    path_env += false_path.string();
    putenv(const_cast<char *>(path_env.data()));

    fs::create_directories({"sandbox/bin/true"});
    fs::create_directories({"sandbox/home/dir1/true"});
    fs::create_directories({"sandbox/home/dir1/dir2"});
  }

  ~PathResolverTest() override {
    fs::current_path(false_root);
    fs::remove_all({"sandbox"});
  }
};

TEST_F(PathResolverTest, does_resolve_filenames) {
  fs::current_path({"sandbox/home/dir1/dir2"});
  auto path = get_absolute_command_path("true");
  ASSERT_TRUE(path);

  auto expected_Path = false_root / fs::path("sandbox/bin/true");
  EXPECT_EQ(*path, expected_Path.string());
}

TEST_F(PathResolverTest, does_resolve_relative_paths) {
  fs::current_path({"sandbox/home/dir1/dir2"});
  auto path1 = get_absolute_command_path("../true");
  ASSERT_TRUE(path1);

  fs::current_path({"../"});
  auto path2 = get_absolute_command_path("./true");
  ASSERT_TRUE(path2);

  auto expected_Path = false_root / fs::path("sandbox/home/dir1/true");
  EXPECT_EQ(*path1, expected_Path.string());
  EXPECT_EQ(*path2, expected_Path.string());
}
