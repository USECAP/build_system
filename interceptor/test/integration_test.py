import json
import os
import subprocess
import unittest

from shutil import which
from pathlib import Path

from build_system.interceptor.grpc_server import InterceptorServer
from build_system.interceptor.intercept_settings import parse_fuzzer_config
from build_system.interceptor.interceptor import main

LD_PRELOAD_PATH = Path("build_system", "preload_interceptor",
                       "preload_interceptor.so").resolve()
INTERCEPT_PATH = os.path.abspath(
    Path("build_system", "interceptor", "intercept"))
TEST_PATH = os.path.abspath(Path("build_system", "interceptor", "test"))
BUILD_SH_PATH = Path(TEST_PATH, "build.sh").resolve()


class IntegrationTests(unittest.TestCase):
    """
    Class for doing the integration tests for the interceptor
    """
    server = None

    def setUp(self):
        yaml_path = "build_system/interceptor/test/mock_cc_config.yaml"
        with open(yaml_path, "r") as f:
            self.server = InterceptorServer(
                intercept_settings=parse_fuzzer_config(f, "mock-cc"))
        self.server.start()
        self.env = os.environ.copy()
        self.env["LD_PRELOAD"] = LD_PRELOAD_PATH
        self.env["REPORT_URL"] = "localhost:{}".format(self.server.port)

    def tearDown(self):
        self.server.stop()

    def test_ld_preload_and_buildfile_exists(self):
        """Check if the preload and build files even exists"""
        self.assertTrue(
            LD_PRELOAD_PATH.is_file(),
            msg="""
                        LD_PRELOAD lib not found
                        current wd: {}
                        current wd contents: {}""".format(
                os.getcwd(), os.listdir(os.getcwd())))
        self.assertTrue(
            os.path.isfile(BUILD_SH_PATH),
            msg="""
                        build.sh not found
                        current wd: {}
                        current wd contents: {}""".format(
                os.getcwd(), os.listdir(os.getcwd())))

    def test_preload_lib_requests_settings(self):
        """Loads the preload through environment and checks whether the
        settings were retrieved"""
        subprocess.call([str(BUILD_SH_PATH)], cwd=str(TEST_PATH), env=self.env)
        self.assertTrue(
            self.server.interceptor_service.GET_INTERCEPT_SETTINGS_WAS_CALLED,
            msg="Preload Lib did not get settings!")

    def test_receive_commands(self):
        """Checks whether the interceptor is able to receive commands"""
        returncode = subprocess.call(
            [str(BUILD_SH_PATH)], cwd=str(TEST_PATH), env=self.env)
        cmds = self.server.interceptor_service.cmds
        self.assertEqual(returncode, 0, msg="Build couldn't be executed")
        self.assertTrue(cmds, "Commands are empty")

        cmd = self.server.interceptor_service.cmds.get_nowait()
        path_to_mock_cc = os.path.realpath(which('mock-cc'))
        self.assertEqual(cmd['replaced_command'], path_to_mock_cc)
        self.assertListEqual(
            list(cmd['replaced_arguments']),
            ['mock-cc', 'hello.c', '-o', 'foo', '-g', '-O2'])

    def test_intercept_method(self):
        returncode = main(["--", str(BUILD_SH_PATH)], cwd=str(TEST_PATH))
        self.assertEqual(
            returncode,
            0,
            msg="interceptor's main method couldn't be executed")

    def test_intercept_script(self):
        proc = subprocess.run(
            [INTERCEPT_PATH, "--", str(BUILD_SH_PATH)],
            cwd=str(TEST_PATH),
            stdout=subprocess.PIPE)
        self.assertEqual(proc.returncode, 0, msg="intercept not executed")

    def test_intercept_parser_argument(self):
        cmd = ["--fuzzer", "libfuzzer", "--", str(BUILD_SH_PATH)]
        returncode = main(cmd, cwd=str(TEST_PATH))
        self.assertEqual(returncode, 0, msg="intercept not executed")

    def test_intercept_compilation_db(self):
        returncode = main(
            ["--create_compiler_database", "--",
             str(BUILD_SH_PATH)],
            cwd=str(TEST_PATH))
        self.assertTrue(Path(TEST_PATH, "compile_commands.json").is_file())
        # compilation db must be valid json
        with open(Path(TEST_PATH, "compile_commands.json"), "r") as file:
            self.assertTrue(json.load(file))
        self.assertEqual(returncode, 0, msg="intercept not executed")


if __name__ == '__main__':
    os.environ['PATH'] = str(TEST_PATH) + ':' + os.environ['PATH']
    unittest.main()
