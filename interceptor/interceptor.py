"""
Module for intercepting exec calls
"""
import argparse
import asyncio
import json
import os
import sys

from pathlib import Path

from build_system.interceptor import intercept_settings
from build_system.interceptor.grpc_server import InterceptorServer
from build_system.interceptor.commandline import parse_commandline

MODULE_PATH = Path(sys.modules[__name__].__file__).parent
FUZZER_CONFIGS_PATH = Path(MODULE_PATH, 'config', 'fuzzer')
LD_PRELOAD_LIB = Path(MODULE_PATH,
                      "../preload_interceptor/preload_interceptor.so")


class CompilationDatabase:
    """CompilationDatabase for creating clang's compilation database
    compile_commands.json"""

    DEFAULT_COMPILATION_DB = "compile_commands.json"

    def __init__(self, queue, file):
        """
        Constructor for the CompilationDatabase class
        :param queue: the que where the commands come from
        :param cwd: the directory where the database is created (default is
                    the current working directory
        :param file: file descriptor for the database (recommended name is
                     in the call constant DEFAULT_COMPILATION_DB which equals
                     "compilation_database.json")
        """
        self.db = file
        self.queue = queue
        self._is_first_iteration = True

    async def write_compile_commands_db(self):
        """
        Asynchronous method to actually write the compilation db

        """
        self.db.write("[\n")
        while True:
            cmd = await self.queue.get()
            if cmd is None:
                break
            command_name = cmd['replaced_command']
            # arguments without argv[0] (the command name again)
            command_args = cmd['replaced_arguments'][1:]
            command_line = [command_name] + command_args
            compile_file = parse_commandline(" ".join(command_line))
            for input_file in compile_file.input_files:
                self._write_entry(
                    arguments=command_line,
                    directory=cmd['directory'],
                    output=compile_file.output_file,
                    file=input_file)
        self.db.write("]\n")

    def _write_entry(self, **kwargs):
        if self._is_first_iteration is not True:
            self.db.write(",\n")
        self._is_first_iteration = False
        json.dump(kwargs, self.db)


class Interceptor:
    """
    Interceptor intercepts all exec commands, filters compiler calls and writes
    those in a compilation database

    :param argv:    argument vector, e.g.: intercept --options -- make
    :param cwd:  specify current working directory if needed
    """

    def __init__(self, argv, cwd=None):
        self._loop = asyncio.get_event_loop()
        self.returncode = -1
        self._setup_arg_parser()
        try:
            split_index = argv.index('--')
        except ValueError:
            self._print_usage()
            sys.exit(-1)
        try:
            self.command = argv[split_index + 1:]
            self.args = self._arg_parser.parse_args(argv[:split_index])
        except IndexError:
            self._print_usage()
            sys.exit(-1)
        if cwd is None:
            self.cwd = Path(os.getcwd())
        else:
            self.cwd = cwd
        self.server = InterceptorServer(
            intercept_settings=intercept_settings.parse_fuzzer_config(
                self.args.config, self.args.fuzzer))
        self.server.start()
        call = asyncio.ensure_future(self._call())
        if self.args.create_compiler_database:
            path = Path(self.cwd, CompilationDatabase.DEFAULT_COMPILATION_DB)
            with open(path, "w") as file:
                database = CompilationDatabase(
                    self.server.interceptor_service.cmds, file=file)
                db = asyncio.ensure_future(
                    database.write_compile_commands_db())
                self._loop.run_until_complete(db)

        self.returncode = self._loop.run_until_complete(call)
        self.server.stop()

    def __del__(self):
        try:
            self.args.config.close()
        except AttributeError:
            pass

    def _print_usage(self):
        self._arg_parser.print_help()

    def _setup_arg_parser(self):
        self._arg_parser = argparse.ArgumentParser()
        self._arg_parser.add_argument(
            "--fuzzer",
            help="use fuzzer configuration",
            type=str,
            default="default")
        self._arg_parser.add_argument(
            "--match_compiler",
            type=str,
            default="",
            help="compiler call which might be intercepted")
        self._arg_parser.add_argument(
            "--create_compiler_database",
            action="store_true",
            default=False,
            help="whether a compiler database should be created")
        self._arg_parser.add_argument(
            "--config",
            help="use fuzzer configuration",
            type=argparse.FileType("r"),
            default=str(Path(FUZZER_CONFIGS_PATH, "default.yaml")))

    async def _call(self):
        """
        Makes the build process call given by the user (e.g. make)
        :return: returncode of the build process call
        """
        if not LD_PRELOAD_LIB.is_file():
            print("LD_PRELOAD_LIB {} doesn't exist...".format(LD_PRELOAD_LIB))
            return -1
        env = os.environ.copy()
        env["LD_PRELOAD"] = str(LD_PRELOAD_LIB)
        env["REPORT_URL"] = "localhost:{}".format(self.server.port)
        process = await asyncio.create_subprocess_exec(
            *self.command, env=env, cwd=str(self.cwd))
        await process.communicate()

        # write None to queue to communicate that the call finished
        self.server.interceptor_service.cmds.put_nowait(None)

        return process.returncode


def main(argv=sys.argv[1:], cwd=None):
    """
    Main function to execute and intercept the build process given by the user.
    Take note that this function can be called from testcode to increase
    testing coverage.

    :param argv: argument vector without the program name itself
    :param cwd: current working directory
    :return: returncode of the build call
    """
    interceptor = Interceptor(argv, cwd=cwd)

    return interceptor.returncode
