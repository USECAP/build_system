import unittest
from build_system.interceptor.split_compile_command import SplitCompileCommand


class SplitArgvTests(unittest.TestCase):
    """
    Test class to test the compilation argv interpreter
    """
    CMD = "gcc -I /usr/include -I /usr/local/include -D LINUX -O3 -c main.c"

    def test_single_file(self):
        self.SplitCompileCommand = SplitCompileCommand(self.CMD)

        self.assertEqual(
            self.SplitCompileCommand.get_source_file(), ['main.c']
        )

    def test_multi_files(self):
        self.SplitCompileCommand = SplitCompileCommand(self.CMD + " test.c")

        self.assertEqual(
            self.SplitCompileCommand.get_source_file(), ['main.c', 'test.c']
        )


if __name__ == '__main__':
    unittest.main()
