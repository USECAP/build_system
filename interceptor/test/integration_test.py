import os
import subprocess
import unittest

from pathlib import Path

from build_system.interceptor.grpc_server import InterceptorServer
from build_system.interceptor.interceptor import main

LD_PRELOAD_PATH = Path(
    "build_system", "preload_interceptor", "preload_interceptor.so").resolve()
INTERCEPT_PATH = os.path.abspath(
    Path("build_system", "interceptor", "intercept"))
TEST_PATH = Path("build_system", "interceptor", "test").resolve()
BUILD_SH_PATH = Path(TEST_PATH, "build.sh").resolve()


class IntegrationTests(unittest.TestCase):
    """
    Class for doing the integration tests for the interceptor
    """
    server = None

    def setUp(self):
        self.server = InterceptorServer()
        self.server.start()
        self.env = os.environ.copy()
        self.env["LD_PRELOAD"] = LD_PRELOAD_PATH
        self.env["REPORT_URL"] = "localhost:{}".format(self.server.port)

    def tearDown(self):
        self.server.stop()

    def test_ld_preload_and_buildfile_exists(self):
        """Check if the preload and build files even exists"""
        self.assertTrue(
            LD_PRELOAD_PATH.is_file(), msg="""
                        LD_PRELOAD lib not found
                        current wd: {}
                        current wd contents: {}""".format(
                os.getcwd(), os.listdir(os.getcwd())))
        self.assertTrue(
            BUILD_SH_PATH.is_file(), msg="""
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
        returncode = subprocess.call([str(BUILD_SH_PATH)], cwd=str(TEST_PATH),
                                     env=self.env)
        self.assertEqual(returncode, 0, msg="Build couldn't be executed")
        self.assertTrue(self.server.interceptor_service.cmds,
                        "Commands are empty")

        self.assertEqual(
            self.server.interceptor_service.cmds[1]['replaced_command'],
            '/usr/bin/gcc')
        self.assertListEqual(
            list(
                self.server.interceptor_service.cmds[1]['replaced_arguments']),
            ['hello.c', '-o', 'foo'])

    def test_intercept_method(self):
        returncode = main(["--", BUILD_SH_PATH], cwd=TEST_PATH)
        self.assertEqual(returncode, 0,
                         msg="interceptor's main method couldn't be executed")

    def test_intercept_script(self):
        returncode = subprocess.call([INTERCEPT_PATH, "--", BUILD_SH_PATH],
                                     cwd=str(TEST_PATH))
        self.assertEqual(returncode, 0, msg="intercept not executed")

    def test_intercept_parser_argument(self):
        cmd = [INTERCEPT_PATH, "--fuzzer", "libfuzzer", "--", BUILD_SH_PATH]
        returncode = subprocess.call(cmd, cwd=str(TEST_PATH))
        self.assertEqual(returncode, 0, msg="intercept not executed")


if __name__ == '__main__':
    unittest.main()
