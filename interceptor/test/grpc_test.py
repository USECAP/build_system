import unittest
from build_system.interceptor.grpc_server import InterceptorServer, InterceptorClient


class GrpcTests(unittest.TestCase):
    def setUp(self):
        self.server = InterceptorServer()
        self.server.start()
        self.client = InterceptorClient("localhost", self.server.port)


    def test_server_started(self):
        self.assertTrue(self.client.get_settings())

    def test_send_updates(self):
        response = self.client.send_update("gcc", "-v", "clang", "-v")
        self.assertTrue(response.received == 1)
        self.client.send_update("gcc", "-v", "clang", "-v")
        self.client.send_update("gcc", "-v", "clang", "-v")
        response = self.client.send_update("gcc", "-v", "clang", "-v")
        self.assertEquals(response.received, 4)
        self.assertTrue(response.message == "4 commands received")
        self.assertEquals(response.processed, 0)


if __name__ == '__main__':
    unittest.main()
