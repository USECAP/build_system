import os
import subprocess
import sys

from build_system.interceptor.grpc_server import InterceptorServer, InterceptorService

LD_PRELOAD_LIB = "build_system/preload_interceptor/preload_interceptor.so"
BUILD_SH_PATH = "build_system/interceptor/test/build.sh"

if __name__ == '__main__':
    server = InterceptorServer(InterceptorService())
    server.start()
    if not os.path.isfile(LD_PRELOAD_LIB):
        print("LD_PRELOAD_LIB {} doesn't exist...".format(LD_PRELOAD_LIB))
        sys.exit(-1)
    subprocess.call(sys.argv[1:], env={"LD_PRELOAD": LD_PRELOAD_LIB, "REPORT_URL": server.port})

