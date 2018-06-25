"""
Parse the intercept settings from a YAML into a GRPC / Protobuf
InterceptSettings structure.
"""

import yaml
from build_system.proto.intercept_pb2 import InterceptSettings, MatchingRule

DEFAULT_CC_RE = r"^([^-]*-)*[mg]cc(-\d+(\.\d+){0,2})?$|" \
                r"^([^-]*-)*clang(-\d+(\.\d+){0,2})?$|" \
                r"^(|i)cc$|^(g|)xlc$'"

DEFAULT_CXX_RE = r"^([^-]*-)*[mg]\+\+(-\d+(\.\d+){0,2})?$|" \
                 r"^([^-]*-)*clang\+\+(-\d+(\.\d+){0,2})?$|" \
                 r"^(|i)cc$|^(g|)xlc$'"  # TODO what are the c++ variants here?


class FuzzerNotFound(Exception):
    """
    Exception raised when the fuzzer was not found in the config.
    """


def _match_command(match):
    if match == "DEFAULT_CC":
        return DEFAULT_CC_RE
    if match == "DEFAULT_CXX":
        return DEFAULT_CXX_RE
    return match


def parse_fuzzer_config(data, fuzzer):
    """
    Parse a fuzzer config and return an InterceptSettings object
    :param data: The yaml config to parse
    :param fuzzer: The fuzzer (key) in the yaml config
    :return: InterceptSettings the settings created from the yaml
    """
    configs = yaml.safe_load(data)

    try:
        config = configs[fuzzer]
    except KeyError:
        raise FuzzerNotFound("Fuzzer {} not found.".format(fuzzer))

    return InterceptSettings(matching_rules=[
        MatchingRule(replace_command=tool['replace'],
                     match_command=_match_command(tool['match']),
                     add_arguments=tool['add_arguments'],
                     remove_arguments=tool['remove_arguments']) for
        tool in config['toolchain'].values()
    ])
