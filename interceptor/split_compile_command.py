# -*- coding: utf-8 -*-
# Copyright (C) 2012-2017 by László Nagy
# This code is part of Bear.

import re

from pathlib import Path

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
    """
    The SplitCompileCommand class interprets the command line arguments of a
    compiler call
    """

    def __init__(self, command):
        """
        Constructor
        :param command: compiler command
        """
        self._output_file = None
        self.compiler = None
        self.compile_argv = []
        self._input_files = []

        self._split_compiler(command)
        if not self.compiler:
            return

        self._parse_args()

    @property
    def input_files(self):
        """Extract the file(s) outof the compile command
        :return: a list of sourcefiles. """
        return self._input_files

    @property
    def output_file(self):
        """Return the output file of the compile command."""
        if not self._output_file:
            return 'a.out'
        return self._output_file

    @property
    def is_linker_command(self):
        """Return True iff. the command is a linker command"""
        return False

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

        extension = Path(filename).suffix
        return mapping.get(extension)

    def _split_compiler(self, command):
        """ A predicate to decide whether the command is a compiler call."""
        command = command.split(" ")

        if len(command):
            self.compiler = Path(command[0]).name
            self.compile_argv = command[1:]

    def _parse_args(self):
        """ Returns a value when the command is a compilation, None otherwise.
        :return: a List of source files """

        if self.compile_argv is None:
            return

        output_follows = False
        for arg in self.compile_argv:
            if output_follows:
                output_follows = False
                self._output_file = arg
            # parameter which looks source file is taken...
            if (re.match(r'^[^-].+', arg) and self._classify_source(arg)):
                self._input_files.append(arg)
            if arg == '-o':
                output_follows = True
