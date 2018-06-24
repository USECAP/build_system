# -*- coding: utf-8 -*-
# Copyright (C) 2012-2017 by László Nagy
# This code is part of Bear.

import re
import os


IGNORED_FLAGS = {
    # preprocessor macros, ignored because would cause duplicate entries in
    # the output (the only difference would be these flags). this is actual
    # finding from users, who suffered longer execution time caused by the
    # duplicates.
    '-MD': 0,
    '-MMD': 0,
    '-MG': 0,
    '-MP': 0,
    '-MF': 1,
    '-MT': 1,
    '-MQ': 1,
    # linker options, ignored because for compilation database will contain
    # compilation commands only. so, the compiler would ignore these flags
    # anyway. the benefit to get rid of them is to make the output more
    # readable.
    '-static': 0,
    '-shared': 0,
    '-s': 0,
    '-rdynamic': 0,
    '-l': 1,
    '-L': 1,
    '-u': 1,
    '-z': 1,
    '-T': 1,
    '-Xlinker': 1,
    # clang-cl / msvc cl specific flags
    # consider moving visual studio specific warning flags also
    '-nologo': 0,
    '-EHsc': 0,
    '-EHa': 0

}


class SplitCompileCommand:

    def __init__(self, command):
        self.compiler = ""
        self.compile_argv = []
        self.file = []

        try:
            if self._split_compiler(command) is None:
                raise TypeError
        except TypeError:
            print("bad input ...")
            return

        try:
            self.file = self._split_args()
            if self.file is None:
                raise TypeError
        except TypeError:
            print("sourcefile not detected ...")
            return

    def get_source_file(self):
        """Extract the file(s) outof the compile command
        :return: a list of sourcefiles. """
        return self.file

    def _classify_source(self, filename, c_compiler=True):
        """ Classify source file names and returns the presumed language,
        based on the file name extension.
        :param filename:    the source file name
        :param c_compiler:  indicate that the compiler is a C compiler,
        :return: the language from file name extension. """

        mapping = {
            '.c': 'c' if c_compiler else 'c++',
            '.i': 'c-cpp-output' if c_compiler else 'c++-cpp-output',
            '.ii': 'c++-cpp-output',
            '.m': 'objective-c',
            '.mi': 'objective-c-cpp-output',
            '.mm': 'objective-c++',
            '.mii': 'objective-c++-cpp-output',
            '.C': 'c++',
            '.cc': 'c++',
            '.CC': 'c++',
            '.cp': 'c++',
            '.cpp': 'c++',
            '.cxx': 'c++',
            '.c++': 'c++',
            '.C++': 'c++',
            '.txx': 'c++',
            '.s': 'assembly',
            '.S': 'assembly',
            '.sx': 'assembly',
            '.asm': 'assembly'
        }

        __, extension = os.path.splitext(os.path.basename(filename))
        return mapping.get(extension)

    def _split_compiler(self, command):
        """ A predicate to decide whether the command is a compiler call."""
        command = command.split(" ")

        if len(command):
            self.compiler = os.path.basename(command[0])
            self.compile_argv = command[1:]
            return True
        else:
            self.compiler = None
            self.compile_argv = None
            return None

    def _split_args(self):
        """ Returns a value when the command is a compilation, None otherwise.
        :return: a List of source files """

        result = []

        if self.compile_argv is None:
            return None

        for argument in self.compile_argv:
            # parameter which looks source file is taken...
            if (re.match(r'^[^-].+', argument) and
                    self._classify_source(argument)):
                result.append(argument)

        return result
