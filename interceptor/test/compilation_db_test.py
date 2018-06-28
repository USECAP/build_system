import asyncio
import json
import tempfile
import unittest

from build_system.interceptor.interceptor import CompilationDatabase


class CompilationDatabaseTests(unittest.TestCase):
    """Tests for the CompilationDatabase class"""

    def test_intercept_compilation_db(self):
        """
        Tests the compilation db with just one compiler call
        :return:
        """
        queue = asyncio.Queue()
        with tempfile.TemporaryFile(mode="r+") as temp:
            db = CompilationDatabase(queue, file=temp)
            self.assertTrue(db)

            expected = self.create_commands(queue, 1)

            asyncio.get_event_loop().run_until_complete(
                db.write_compile_commands_db())
            temp.seek(0)

            compilation_db = json.load(temp)
            self.assertEqual(compilation_db, expected)

    def test_intercept_compilation_db_multiple_calls(self):
        """
        Tests the compilation db with just one compiler call
        :return:
        """
        queue = asyncio.Queue()
        with tempfile.TemporaryFile(mode="r+") as temp:
            db = CompilationDatabase(queue, file=temp)
            self.assertTrue(db)

            expected = self.create_commands(queue, 3)

            asyncio.get_event_loop().run_until_complete(
                db.write_compile_commands_db())
            temp.seek(0)

            compilation_db = json.load(temp)
            self.assertEqual(compilation_db, expected)

    def test_compilation_db_multiple_input_files(self):
        """Test that we generate multiple entries when a command touches
        multipe input files.
        """
        queue = asyncio.Queue()
        with tempfile.TemporaryFile(mode="r+") as temp:
            db = CompilationDatabase(queue, file=temp)
            queue.put_nowait({
                'replaced_command':
                "gcc",
                'replaced_arguments': ["gcc", "input1.c", "input2.c"],
                'directory':
                "",
            })
            queue.put_nowait(None)

            asyncio.get_event_loop().run_until_complete(
                db.write_compile_commands_db())
            temp.seek(0)

            compilation_db = json.load(temp)
            self.assertEqual(compilation_db,
                             [{
                                 'arguments': ["gcc", "input1.c", "input2.c"],
                                 'directory': '',
                                 'output': 'a.out',
                                 'file': 'input1.c'
                             }, {
                                 'arguments': ["gcc", "input1.c", "input2.c"],
                                 'directory': '',
                                 'output': 'a.out',
                                 'file': 'input2.c'
                             }])

    @staticmethod
    def create_commands(queue, n):
        command = {
            'replaced_command': "gcc",
            'replaced_arguments': ["gcc", "-o", "foo", "test.c"],
            'directory': "",
        }
        expected = {
            'arguments': ["gcc", "-o", "foo", "test.c"],
            'directory': '',
            'output': 'foo',
            'file': 'test.c'
        }
        for i in range(0, n):
            queue.put_nowait(command)
        queue.put_nowait(None)

        expected = [expected] * n
        return expected


if __name__ == '__main__':
    unittest.main()
