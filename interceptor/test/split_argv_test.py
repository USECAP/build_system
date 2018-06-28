import unittest

from build_system.interceptor.commandline import parse_commandline


class SplitArgvTests(unittest.TestCase):
    """
    Test class to test the compilation argv interpreter
    """
    CMD = "gcc -I /usr/include -I /usr/local/include" \
          "-D LINUX -O3 -c main.c"

    def test_single_file(self):
        parsed_cmd = parse_commandline(self.CMD)
        self.assertEqual(parsed_cmd.input_files, ['main.c'])
        self.assertEqual(parsed_cmd.output_file, 'a.out')
        self.assertFalse(parsed_cmd.is_linker_command)

    def test_output_file(self):
        parsed_cmd = parse_commandline(self.CMD + ' -o main')
        self.assertEqual(parsed_cmd.output_file, 'main')
        self.assertFalse(parsed_cmd.is_linker_command)

    def test_multi_files(self):
        parsed_cmd = parse_commandline(self.CMD + " test.c")
        self.assertEqual(parsed_cmd.input_files, ['main.c', 'test.c'])
        self.assertFalse(parsed_cmd.is_linker_command)

    def test_linker_command(self):
        linker_cmd = "afl-clang -fPIC -std=gnu99 -Qunused-arguments " \
                     "-Werror -Wno-deprecated-declarations -g -shared " \
                     "-Wl,-soname,libacc_preload.so -o libacc_preload.so " \
                     "CMakeFiles/acc_preload.dir/acc_preload.c.o -lzmq -g -O2"
        parsed_cmd = parse_commandline(linker_cmd)
        self.assertEqual(parsed_cmd.output_file, 'libacc_preload.so')
        self.assertTrue(parsed_cmd.is_linker_command)

    def test_simple_naming(self):
        cmd = "gcc -o foo test.c"
        parsed_cmd = parse_commandline(cmd)
        self.assertEqual("foo", parsed_cmd.output_file)


if __name__ == '__main__':
    unittest.main()
