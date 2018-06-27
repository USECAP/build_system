import os
from collections import namedtuple

CommandlineInfo = namedtuple(
    'CommandlineInfo', ['input_files', 'output_file', 'is_linker_command'])
CommandlineInfo.__doc__ = """
CommandlineInfo holds the parsed commandline information.
"""

_LINKER_FLAGS = [
    '-static', '-shared', '-s', '-rdynamic', '-l', '-L', '-u', '-z', '-T',
    '-Xlinker'
]

_OUTPUT_SUFFIXES = [
    '.c', '.i', '.ii', '.m', '.mi', '.mm', '.mii', '.C', '.cc', '.CC', '.cp',
    '.cpp', '.cxx', '.c++', '.C++', '.txx', '.s', '.S', '.sx', '.asm'
]


def _arguments(command):
    """ A predicate to decide whether the command is a compiler call."""
    command = command.split(" ")
    if len(command):
        return command[1:]


def _parse_arguments(arguments):
    """ Returns a value when the command is a compilation, None otherwise.
    :return: a List of source files """

    info = {
        "input_files": [],
        "output_file": "a.out",
        "is_linker_command": False
    }

    output_follows = False

    for arg in arguments:
        # Handle output
        if arg == '-o':
            output_follows = True
            continue
        if output_follows:
            output_follows = False
            info["output_file"] = arg

        # Handle input
        _, extension = os.path.splitext(arg)
        if extension in _OUTPUT_SUFFIXES:
            info["input_files"].append(arg)

        # Handle linker flag
        if arg in _LINKER_FLAGS:
            info["is_linker_command"] = True

    return CommandlineInfo(**info)


def parse_commandline(commandline):
    """
    Parse a commandline (usually from a compiler invocation) and output a
    structured result with input_files, output_file and is_linker_command
    :param commandline: a commandline string
    :return: a CommandlineInfo object
    """
    arguments = _arguments(commandline)
    if arguments is None:
        return CommandlineInfo(
            input_files=[], output_file="", is_linker_command=False)

    return _parse_arguments(arguments)
