// Copyright (c) 2018 Code Intelligence. All rights reserved.

#pragma once

#include <map>
#include <string>
#include "absl/strings/string_view.h"

struct ArgInfo {
  int arity;
};

static std::map<absl::string_view, ArgInfo> CC_ARGUMENTS_INFO = {
    {"-", {0}},
    {"-o", {1}},
    {"-c", {0}},
    {"-E", {0}},
    {"-S", {0}},
    {"--verbose", {0}},
    {"--param", {1}},
    {"-aux-info", {1}},

    // iam: presumably the len(inputFiles) == 0 in this case
    {"--version", {0}},
    {"-v", {0}},

    // warnings (apart from the regex below)
    {"-w", {0}},
    {"-W", {0}},

    // iam: if this happens, then we need to stop and think.
    {"-emit-llvm", {0}},

    // iam: buildworld and buildkernel use these flags
    {"-pipe", {0}},
    {"-undef", {0}},
    {"-nostdinc", {0}},
    {"-nostdinc++", {0}},
    {"-Qunused-arguments", {0}},
    {"-no-integrated-as", {0}},
    {"-integrated-as", {0}},

    // iam: gcc uses this in both compile and link, but clang only in compile
    {"-pthread", {0}},

    // I think this is a compiler search path flag.  It is clang only, so I
    // don't think it counts as a separate CPPflag. Android uses this flag with
    // its clang builds.
    {"-nostdlibinc", {0}},

    // iam: arm stuff
    {"-mno-omit-leaf-frame-pointer", {0}},
    {"-maes", {0}},
    {"-mno-aes", {0}},
    {"-mavx", {0}},
    {"-mno-avx", {0}},
    {"-mcmodel=kernel", {0}},
    {"-mno-red-zone", {0}},
    {"-mmmx", {0}},
    {"-mno-mmx", {0}},
    {"-msse", {0}},
    {"-mno-sse2", {0}},
    {"-msse2", {0}},
    {"-mno-sse3", {0}},
    {"-msse3", {0}},
    {"-mno-sse", {0}},
    {"-msoft-float", {0}},
    {"-m3dnow", {0}},
    {"-mno-3dnow", {0}},
    {"-m16", {0}},
    {"-m32", {0}},
    {"-mx32", {0}},
    {"-m64", {0}},
    {"-miamcu", {0}},
    {"-mstackrealign", {0}},
    {"-mretpoline-external-thunk", {0}},
    {"-mno-fp-ret-in-387", {0}},
    {"-mskip-rax-setup", {0}},
    {"-mindirect-branch-register", {0}},

    // Preprocessor assertion
    {"-A", {1}},
    {"-D", {1}},
    {"-U", {1}},

    // Dependency generation
    {"-M", {0}},
    {"-MM", {0}},
    {"-MF", {1}},
    {"-MG", {0}},
    {"-MP", {0}},
    {"-MT", {1}},
    {"-MQ", {1}},
    {"-MD", {0}},
    {"-MMD", {0}},

    // Include
    {"-I", {1}},
    {"-idirafter", {1}},
    {"-include", {1}},
    {"-imacros", {1}},
    {"-iprefix", {1}},
    {"-iwithprefix", {1}},
    {"-iwithprefixbefore", {1}},
    {"-isystem", {1}},
    {"-isysroot", {1}},
    {"-iquote", {1}},
    {"-imultilib", {1}},

    // Language
    {"-ansi", {0}},
    {"-pedantic", {0}},

    // iam: i notice that yices configure passes -xc so we should have a fall
    // back pattern that captures the case when
    // there is no space between the x and the langauge. for what its worth: the
    // manual says the language can be one of c
    //  objective-c  c++ c-header  cpp-output  c++-cpp-output assembler
    //  assembler-with-cpp
    // BD: care to comment on your configure?
    {"-x", {1}},

    // Debug
    {"-g", {0}},
    {"-g0", {0}},
    {"-ggdb", {0}},
    {"-ggdb3", {0}},
    {"-gdwarf-2", {0}},
    {"-gdwarf-3", {0}},
    {"-gline-tables-only", {0}},

    {"-p", {0}},
    {"-pg", {0}},

    // Optimization
    {"-O", {0}},
    {"-O0", {0}},
    {"-O1", {0}},
    {"-O2", {0}},
    {"-O3", {0}},
    {"-Os", {0}},
    {"-Ofast", {0}},
    {"-Og", {0}},

    // Component-specifiers
    {"-Xclang", {1}},
    {"-Xpreprocessor", {1}},
    {"-Xassembler", {1}},
    {"-Xlinker", {1}},

    // Linker
    {"-l", {1}},
    {"-L", {1}},
    {"-T", {1}},
    {"-u", {1}},

    // iam: specify the entry point
    {"-e", {1}},

    // runtime library search path
    {"-rpath", {1}},

    // iam: showed up in buildkernel
    {"-shared", {0}},
    {"-static", {0}},
    {"-pie", {0}},
    {"-nostdlib", {0}},
    {"-nodefaultlibs", {0}},
    {"-rdynamic", {0}},

    // darwin flags
    {"-dynamiclib", {0}},
    {"-current_version", {1}},
    {"-compatibility_version", {1}},

    // dragonegg mystery argument
    {"--64", {0}},

    // binutils nonsense
    {"-print-multi-directory", {0}},
    {"-print-multi-lib", {0}},
    {"-print-libgcc-file-name", {0}},

    // Code coverage instrumentation
    {"-fprofile-arcs", {0}},
    {"-coverage", {0}},
    {"--coverage", {0}},

    // ian's additions while building the linux kernel
    {"/dev/null", {0}},
    {"-mno-80387", {0}},

    // BD: need to warn the darwin user that these flags will rain on their
    // parade (the Darwin ld is a bit single minded) 1) compilation with
    // -fvisibility=hidden causes trouble when we try to attach bitcode
    // filenames to an object file. The global symbols in object files get
    // turned into local symbols when we invoke 'ld -r' 2) all stripping
    // commands (e.g., -dead_strip) remove the __LLVM segment after linking
    // Update: found a fix for problem 1: add flag -keep_private_externs when
    // calling ld -r.
    {"-Wl,-dead_strip", {0}},
    {"-Oz", {0}},
    {"-mno-global-merge", {0}},
};