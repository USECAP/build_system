import time
from collections import deque

import grpc
from concurrent import futures

from build_system.proto import intercept_pb2
from build_system.proto import intercept_pb2_grpc

GET_INTERCEPT_SETTINGS_WAS_CALLED = False


class InterceptorServer(object):
    """
    gRPC Server to intercept commands via preload library

    :return: object
    """

    def __init__(self, interceptor_service=None, server_port=0, **kwargs):

        if not interceptor_service:
            self.interceptor_service = InterceptorService(**kwargs)
        else:
            self.interceptor_service = interceptor_service

        self.server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
        intercept_pb2_grpc.add_InterceptorServicer_to_server(
            self.interceptor_service, self.server)
        self.port = self.server.add_insecure_port(
            '[::]:{server_port}'.format(server_port=server_port))

    def start(self):
        self.server.start()

    def stop(self):
        self.server.stop(0)

    def await_termination(self):
        """
        server.start() doesn't block so we explicitly block here unless someone
        keyboard-exits us.
        :return:
        """
        try:
            while True:
                time.sleep(60 * 60 * 24)
        except KeyboardInterrupt:
            self.server.stop(0)
        pass


class InterceptorClient(object):

    def __init__(self, host, port):
        self.channel = grpc.insecure_channel("{}:{}".format(host, port))

    def get_settings(self):
        stub = intercept_pb2_grpc.InterceptorStub(self.channel)
        return stub.GetInterceptSettings(
            intercept_pb2.InterceptSettingsRequest())

    def send_update(self, original_command, original_arguments,
                    replaced_command, replaced_arguments):
        stub = intercept_pb2_grpc.InterceptorStub(self.channel)
        commands = [
            intercept_pb2.InterceptedCommand(original_command=original_command,
                                             original_arguments=original_arguments,
                                             replaced_command=replaced_command,
                                             replaced_arguments=replaced_arguments)
        ]
        response = stub.ReportInterceptedCommand(iter(commands))
        return response


class InterceptorService(intercept_pb2_grpc.InterceptorServicer):
    # Known C compiler executable name patterns.
    COMPILER_PATTERNS_CC = '''
        ^([^-]*-)*[mg]cc(-\d+(\.\d+){0,2})?$|
        ^([^-]*-)*clang(-\d+(\.\d+){0,2})?$|
        ^(|i)cc$|^(g|)xlc$'
    '''

    GET_INTERCEPT_SETTINGS_WAS_CALLED = False

    def __init__(self, **kwargs):
        self.cmds = deque()
        self.received = 0

    def GetInterceptSettings(self, request, context,
                             match_command=COMPILER_PATTERNS_CC,
                             replace_command=None,
                             add_arguments=None,
                             remove_arguments=None):
        self.GET_INTERCEPT_SETTINGS_WAS_CALLED = True
        matching_rules = [
            intercept_pb2.MatchingRule(
                match_command=match_command, replace_command=replace_command,
                add_arguments=add_arguments, remove_arguments=remove_arguments)
        ]
        return intercept_pb2.InterceptSettings(matching_rules=matching_rules)

    def ReportInterceptedCommand(self, request_iterator, context):
        for cmd in request_iterator:
            self.cmds.append(
                {'original_command': str(cmd.original_command),
                 'original_arguments': list(cmd.original_arguments),
                 'replaced_command': str(cmd.replaced_command),
                 'replaced_arguments': list(cmd.replaced_arguments),
                 'directory': str(cmd.directory),
                 'file': str(cmd.file),
                 'output': str(cmd.output)})
            self.received += 1

        return intercept_pb2.Status(
            received=self.received, processed=self.received - len(self.cmds),
            message="{} commands received".format(len(self.cmds)))
