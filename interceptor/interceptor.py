"""
Module for intercepting exec calls
"""
import argparse
import json
import os
import subprocess
import sys

from pathlib import Path

from build_system.interceptor import intercept_settings
from build_system.interceptor.grpc_server import InterceptorServer
from build_system.interceptor.split_compile_command import SplitCompileCommand

MODULE_PATH = Path(sys.modules[__name__].__file__).parent
FUZZER_CONFIGS_PATH = Path(MODULE_PATH, 'config', 'fuzzer')
LD_PRELOAD_LIB = Path(MODULE_PATH,
                      "../preload_interceptor/preload_interceptor.so")


class Interceptor:
    """
    Interceptor intercepts all exec commands, filters compiler calls and writes
    those in a compilation database


    :param argv:    argument vector, e.g.: intercept --options -- make
    :param cwd:  specify current working directory if needed
    """

    def __init__(self, argv, cwd=None):
        self.returncode = -1
        try:
            split_index = argv.index('--')
            self.command = argv[split_index + 1:]
            self.args = self._parse_arguments(argv[:split_index])
        except IndexError:
            self.args = None
            print("Wrong usage")
            sys.exit(-1)

        if cwd is None:
            self.cwd = Path(os.getcwd())
        else:
            self.cwd = cwd
        self.server = InterceptorServer(
            intercept_settings=intercept_settings.parse_fuzzer_config(
                self.args.config, self.args.fuzzer))
        self.server.start()
        self.returncode = self._call()
        self.server.stop()
        if self.args.create_compiler_database:
            self._write_compile_commands_db()

    def __del__(self):
        if self.args and self.args.config:
            self.args.config.close()

    def _parse_arguments(self, argv):
        """
        :param argv: parses the raw argv vector
        :return: parsed arguments
        """
        parser = argparse.ArgumentParser()
        parser.add_argument(
            "--fuzzer",
            help="use fuzzer configuration", type=str,
            default="default")
        parser.add_argument(
            "--match_compiler",
            type=str, default="",
            help="compiler call which might be intercepted")
        parser.add_argument(
            "--create_compiler_database",
            type=bool, default=False,
            help="whether a compiler database should be created")
        parser.add_argument(
            "--config",
            help="use fuzzer configuration",
            type=argparse.FileType("r"),
            default=str(Path(FUZZER_CONFIGS_PATH, "default.yaml")))
        return parser.parse_args(argv)

    def _call(self):
        """
        Makes the build process call given by the user (e.g. make)
        :return: returncode of the build process call
        """
        if not LD_PRELOAD_LIB.is_file():
            print("LD_PRELOAD_LIB {} doesn't exist...".format(LD_PRELOAD_LIB))
            return -1
        env = os.environ.copy()
        env["LD_PRELOAD"] = LD_PRELOAD_LIB
        env["REPORT_URL"] = "localhost:{}".format(self.server.port)
        returncode = subprocess.call(self.command, cwd=self.cwd, env=env)
        return returncode

    def _write_compile_commands_db(self):
        with open("{}/compile_commands.json".format(self.cwd), 'w') as file:
            for cmd in self.server.interceptor_service.cmds:
                compile_file = SplitCompileCommand(self.command)
                cmd = {'arguments': [cmd['replaced_command']] + cmd[
                    'replaced_arguments'],
                       'directory': cmd['directory'],
                       'output': cmd['output'],
                       'file': compile_file.get_source_file()}
                json.dump(cmd, file)


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
