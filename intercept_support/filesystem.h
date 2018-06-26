// Copyright (c) 2018 Code Intelligence. All rights reserved.

#pragma once

#ifdef CXX17
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
