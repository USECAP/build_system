"""
Module for the gRPC Server for exec interception
"""
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

    def __init__(self, intercept_settings=None, interceptor_service=None,
                 server_port=0):
        if not intercept_settings:
            intercept_settings = intercept_pb2.InterceptSettings()
        if not interceptor_service:
            self.interceptor_service = InterceptorService(intercept_settings)
        else:
            self.interceptor_service = interceptor_service

        self.server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
        intercept_pb2_grpc.add_InterceptorServicer_to_server(
            self.interceptor_service, self.server)
        self.port = self.server.add_insecure_port(
            '[::]:{server_port}'.format(server_port=server_port))

    def start(self):
        """
        Starts the gRPC server
        :return:
        """
        self.server.start()

    def stop(self):
        """
        Stops the gRPC server
        :return:
        """
        self.server.stop(0)

    def await_termination(self):
        """
        server.start() doesn't block so we explicitly block here unless someone
        keyboard-exits us.
        This is needed if the server is started on the command line
        :return:
        """
        try:
            while True:
                time.sleep(60 * 60 * 24)
        except KeyboardInterrupt:
            self.server.stop(0)
        pass


class InterceptorClient(object):
    """
    Interceptor client to simulate calls (needed by testing)
    :param host: host on which the gRPC server is running
    :param port: port on which the gRPC server is running
    """

    def __init__(self, host, port):
        self.channel = grpc.insecure_channel("{}:{}".format(host, port))

    def get_settings(self):
        """
        Command to get the InterceptSettings from the server
        :return:
        """
        stub = intercept_pb2_grpc.InterceptorStub(self.channel)
        return stub.GetInterceptSettings(
            intercept_pb2.InterceptSettingsRequest())

    def send_update(self, original_command, original_arguments,
                    replaced_command, replaced_arguments):
        """
        Sends periodical updates to server
        :param original_command: original command, e.g., gcc
        :param original_arguments: e.g. -I foo
        :param replaced_command:  e.g. clang
        :param replaced_arguments: e.g. -fsanitize=address
        :return:
        """
        stub = intercept_pb2_grpc.InterceptorStub(self.channel)
        command = intercept_pb2.InterceptedCommand(
            original_command=original_command,
            original_arguments=original_arguments,
            replaced_command=replaced_command,
            replaced_arguments=replaced_arguments)

        response = stub.ReportInterceptedCommand(command)
        return response


class InterceptorService(intercept_pb2_grpc.InterceptorServicer):
    """
    The service which is used by the gRPC server
    """

    # To test whether the settings have been retrieved by the client at least
    # once
    GET_INTERCEPT_SETTINGS_WAS_CALLED = False

    def __init__(self, intercept_settings):
        """
        Constructor
        :param intercept_settings: Settings for the interceptor
        """
        self._intercept_settings = intercept_settings
        self.cmds = deque()
        self.received = 0

    def GetInterceptSettings(self, request, context):
        """
        Get the InterceptSettings
        :param request: request object
        :param context: context object
        :return: InterceptSettings
        """
        self.GET_INTERCEPT_SETTINGS_WAS_CALLED = True
        return self._intercept_settings

    def ReportInterceptedCommand(self, command, context):
        """
        Method called if an update is sent
        :param command: the executed command details
        :param context:
        :return: Status message
        """
        self.cmds.append(
            {'original_command': str(command.original_command),
             'original_arguments': list(command.original_arguments),
             'replaced_command': str(command.replaced_command),
             'replaced_arguments': list(command.replaced_arguments),
             'directory': str(command.directory),
             })
        self.received += 1

        return intercept_pb2.Status(
            received=self.received, processed=self.received - len(self.cmds),
            message="{} commands received".format(len(self.cmds)))
