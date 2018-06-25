"""
Unit tests for intercept_settings.
"""
import re
import unittest

from build_system.interceptor.intercept_settings import parse_fuzzer_config, \
    FuzzerNotFound


class InterceptSettingsTests(unittest.TestCase):
    """
    Test suite for intercept_settings.
    """

    TEST_CONFIG = "build_system/interceptor/test/test_fuzzer_config.yaml"

    def test_throw_on_nonexisting_fuzzer(self):
        with open(self.TEST_CONFIG, "r") as data:
            with self.assertRaises(FuzzerNotFound):
                parse_fuzzer_config(data, "nonexisting_fuzzer")

    def test_get_fuzzer_config(self):
        with open(self.TEST_CONFIG, "r") as data:
            settings = parse_fuzzer_config(data, "my-fuzzer")

        self.assertEqual(len(settings.matching_rules), 2)

        cc_rule = settings.matching_rules[0]

        self.assertEqual(cc_rule.replace_command, "my-cc-fuzzer")

        self.assertTrue(re.fullmatch(cc_rule.match_command, "gcc"))
        self.assertTrue(re.fullmatch(cc_rule.match_command, "clang-6.0"))

        self.assertFalse(re.fullmatch(cc_rule.match_command, "g++"))
        self.assertFalse(re.fullmatch(cc_rule.match_command, "clang++"))

        self.assertTrue(cc_rule.add_arguments)
        self.assertTrue(cc_rule.remove_arguments)

        cxx_rule = settings.matching_rules[1]

        self.assertEqual(cxx_rule.replace_command, "my-c++-fuzzer")

        self.assertFalse(re.fullmatch(cxx_rule.match_command, "gcc"))
        self.assertFalse(re.fullmatch(cxx_rule.match_command, "clang-6.0"))

        self.assertTrue(re.fullmatch(cxx_rule.match_command, "g++"))
        self.assertTrue(re.fullmatch(cxx_rule.match_command, "clang++"))


if __name__ == '__main__':
    unittest.main()
