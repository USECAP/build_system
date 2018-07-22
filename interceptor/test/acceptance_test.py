import os
import unittest
from pathlib import Path
from build_system.interceptor.interceptor import main

TEST_PATH = os.path.abspath(Path("build_system", "interceptor", "test"))
BUILD_SH_PATH = Path(TEST_PATH, "build.sh").resolve()
LOG_PATH = Path(TEST_PATH) / "mock_cc.log"
YAML_PATH = Path(TEST_PATH) / "mock_cc_config.yaml"


class AcceptanceTests(unittest.TestCase):
    """
    Class for doing the acceptance tests for the interceptor.
    """

    def setUp(self):
        if LOG_PATH.exists():
            os.remove(str(LOG_PATH))

    def test_replaced_command_gets_executed(self):
        cmd = [
            "--fuzzer", "mock-cc", "--config",
            str(YAML_PATH), "--",
            str(BUILD_SH_PATH)
        ]

        return_code = main(cmd, cwd=str(TEST_PATH))
        self.assertEqual(
            return_code,
            0,
            msg="interceptor's main method couldn't be executed")
        self.assertTrue(LOG_PATH.exists())

        with open(str(LOG_PATH), "r") as f:
            line = f.readline()
            path_to_mock_cc = os.path.realpath(TEST_PATH + "/mock-cc")
            self.assertEqual(line.strip(),
                             str(path_to_mock_cc) + " hello.c -o foo -g -O2")


if __name__ == '__main__':
    os.environ['PATH'] = str(TEST_PATH) + ':' + os.environ['PATH']
    unittest.main()
