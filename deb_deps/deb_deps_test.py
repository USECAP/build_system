import subprocess
import unittest


class DebDepTests(unittest.TestCase):
    def test_get_dependencies(self):
        proc = subprocess.run(
            ["build_system/deb_deps/deb_deps", "test", "libmp3lame.so"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
        print(proc.stderr.decode())
        output = proc.stdout.decode().strip()
        self.assertIn("libmp3lame-dev", output)
        self.assertIn("libmp3lame0", output)


if __name__ == '__main__':
    unittest.main()
