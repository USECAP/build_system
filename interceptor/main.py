"""
File from which the intercept command is generated
"""
import sys

from build_system.interceptor.interceptor import main

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
