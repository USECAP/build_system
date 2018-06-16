import os
import subprocess
import unittest

from build_system.proto import intercept_pb2
from build_system.proto import intercept_pb2_grpc

from build_system.interceptor.grpc_server import InterceptorServer, InterceptorService

LD_PRELOAD_PATH = "build_system/preload_interceptor/preload_interceptor.so"
BUILD_SH_PATH = "build_system/interceptor/test/build.sh"

GET_INTERCEPT_SETTINGS_WAS_CALLED = False


class IntegrationTests(unittest.TestCase):
    server = None

    def setUp(self):
        self.server = InterceptorServer()
        self.server.start()

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
        self.assertTrue(os.path.isfile(BUILD_SH_PATH), msg="build file doesn't exist")
        subprocess.call([BUILD_SH_PATH], env={"LD_PRELOAD": LD_PRELOAD_PATH,
                                              "REPORT_URL": "localhost:{:d}".format(self.server.port)})
        self.assertTrue(self.server.interceptor_service.GET_INTERCEPT_SETTINGS_WAS_CALLED,
                        msg="Preload Lib did not get settings!")


if __name__ == '__main__':
    unittest.main()

