import argparse
import json
import os
import subprocess
import sys

import yaml

from build_system.interceptor.grpc_server import InterceptorServer

MODULE_PATH = os.path.dirname(sys.modules[__name__].__file__)
FUZZER_CONFIGS_PATH = os.path.join(MODULE_PATH, 'config', 'fuzzer')
LD_PRELOAD_LIB = os.path.join(MODULE_PATH,
                              "../preload_interceptor/preload_interceptor.so")


class Interceptor:
    def __init__(self, argv, cwd=None):
        self.returncode = -1
        self.settings = {}
        try:
            split_index = argv.index('--')
            self.command = argv[split_index + 1:]
            self._parse_arguments(argv[:split_index])
        except IndexError:
            self.args = None
            print("Wrong usage")
            sys.exit(-1)

        if cwd is None:
            self.cwd = os.path.abspath(os.getcwd())
        else:
            self.cwd = cwd
        self.server = InterceptorServer(**self.settings)
        self.server.start()
        self._parse_settings()
        self.returncode = self._call()
        if self.args.create_compiler_database:
            self._write_compile_commands_db()

    def _parse_arguments(self, argv):
        parser = argparse.ArgumentParser()
        parser.add_argument("--fuzzer",
                            help="use fuzzer configuration", type=str,
                            default="default")
        parser.add_argument("--match_compiler",
                            type=str, default="",
                            help="compiler call which might be intercepted")
        parser.add_argument("--create_compiler_database",
                            type=bool, default=False,
                            help="whether a compiler database should be created")
        parser.add_argument(
            "--config",
            help="use fuzzer configuration",
            type=argparse.FileType("r"),
            default=os.path.join(FUZZER_CONFIGS_PATH, "default.yaml"))
        self.args = parser.parse_args(argv)

    def _parse_fuzzer_config(self):
        config = yaml.load(self.args.config)
        config = config[self.args.fuzzer]
        self.settings["replace_command"] = config["compiler"]
        self.settings["add_arguments"] = config["add_arguments"]
        self.settings["remove_arguments"] = config["remove_arguments"]
        return self.settings

    def _parse_settings(self):
        self._parse_fuzzer_config()
        if self.args.match_compiler:
            self.settings["match_command"] = self.args.match_compiler

    def _call(self):
        if not os.path.isfile(LD_PRELOAD_LIB):
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
                cmd = {'arguments': [cmd['replaced_command']] + cmd['replaced_arguments'],
                       'directory': cmd['directory'],
                       'output': cmd['output'],
                       'file': cmd['file']}
                json.dump(cmd, file)


def main(argv=sys.argv[1:], cwd=None):
    interceptor = Interceptor(argv, cwd=cwd)

    return interceptor.returncode
