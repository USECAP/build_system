import os
import subprocess
import unittest

from build_system.interceptor.grpc_server import InterceptorServer
from build_system.interceptor.interceptor import main

LD_PRELOAD_PATH = os.path.abspath(
    "build_system/preload_interceptor/preload_interceptor.so")
INTERCEPT_PATH = os.path.abspath("build_system/interceptor/intercept")
TEST_PATH = os.path.abspath("build_system/interceptor/test")
BUILD_SH_PATH = "{}/{}".format(TEST_PATH, "build.sh")


class IntegrationTests(unittest.TestCase):
    server = None

    def setUp(self):
        self.server = InterceptorServer()
        self.server.start()
        self.env = os.environ.copy()
        self.env["LD_PRELOAD"] = LD_PRELOAD_PATH
        self.env["REPORT_URL"] = "localhost:{}".format(self.server.port)

    def tearDown(self):
        self.server.stop()

    def test_ld_preload_exists(self):
        self.assertTrue(os.path.isfile(LD_PRELOAD_PATH), msg="""
                        LD_PRELOAD lib not found
                        current wd: %s
                        current wd contents: %s""" % (
            os.getcwd(), os.listdir(os.getcwd())))

    def test_build_sh_exists(self):
        self.assertTrue(os.path.isfile(BUILD_SH_PATH), msg="""
                        build.sh not found
                        current wd: %s
                        current wd contents: %s""" % (
            os.getcwd(), os.listdir(os.getcwd())))

    def test_preload_lib_requests_settings(self):
        self.assertTrue(os.path.isfile(BUILD_SH_PATH),
                        msg="build file doesn't exist")
        subprocess.call([BUILD_SH_PATH], cwd=TEST_PATH, env=self.env)
        self.assertTrue(
            self.server.interceptor_service.GET_INTERCEPT_SETTINGS_WAS_CALLED,
            msg="Preload Lib did not get settings!")

    def test_receive_commands(self):
        self.assertTrue(os.path.isfile(BUILD_SH_PATH),
                        msg="build file doesn't exist")
        returncode = subprocess.call([BUILD_SH_PATH], cwd=TEST_PATH,
                                     env=self.env)
        self.assertEqual(returncode, 0, msg="Build couldn't be executed")
        self.assertTrue(self.server.interceptor_service.cmds,
                        "Commands are empty")

        self.assertEqual(
            self.server.interceptor_service.cmds[1]['replaced_command'],
            u'/usr/bin/gcc')
        self.assertListEqual(
            list(self.server.interceptor_service.cmds[1]['replaced_arguments']),
            [u'hello.c', u'-o', u'foo'])

    def test_intercept_method(self):
        returncode = main(["--", BUILD_SH_PATH], cwd=TEST_PATH)
        self.assertEqual(returncode, 0,
                         msg="interceptor's main method couldn't be executed")

    def test_intercept_script(self):
        returncode = subprocess.call([INTERCEPT_PATH, "--", BUILD_SH_PATH],
                                     cwd=TEST_PATH)
        self.assertEqual(returncode, 0, msg="intercept not executed")

    def test_intercept_parser_argument(self):
        cmd = [INTERCEPT_PATH, "--fuzzer", "libfuzzer", "--", BUILD_SH_PATH]
        returncode = subprocess.call(cmd, cwd=TEST_PATH)
        self.assertEqual(returncode, 0, msg="intercept not executed")


if __name__ == '__main__':
    unittest.main()
