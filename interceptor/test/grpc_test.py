import unittest

from build_system.interceptor.grpc_server import (InterceptorServer,
                                                  InterceptorClient)


class GrpcTests(unittest.TestCase):
    """
    Tests the gRPC server/clients
    """

    def setUp(self):
        self.server = InterceptorServer()
        self.server.start()
        self.client = InterceptorClient("localhost", self.server.port)

    def test_server_started(self):
        """Checks whether the server is started and settings can be received"""
        self.assertTrue(self.client.get_settings())

    def test_send_updates(self):
        """Tests whether updates can be sent"""
        response = self.client.send_update("gcc", "-v", "clang", "-v")
        self.assertTrue(response.received == 1)
        self.client.send_update("gcc", "-v", "clang", "-v")
        self.client.send_update("gcc", "-v", "clang", "-v")
        response = self.client.send_update("gcc", "-v", "clang", "-v")
        self.assertEqual(response.received, 4)
        self.assertTrue(response.message == "4 commands received")
        self.assertEqual(response.processed, 0)
        self.assertEqual(self.server.interceptor_service.cmds.qsize(), 4)


if __name__ == '__main__':
    unittest.main()
