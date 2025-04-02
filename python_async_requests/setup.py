from setuptools import setup, Extension
from distutils.command.build_ext import build_ext
import os


class AhrBuilder(build_ext):
    
    def get_ext_filename(self, ext_name):
        return os.path.join(*ext_name.split('.')) + '.so'

    pass

setup(
    cmdclass={
        'build_ext': AhrBuilder,
    },
    include_dirs=[
        'async_http_requests/async_http_requests/inc/async_http_requests/async_http_requests.h',
        'async_http_requests/async_http_requests/inc/async_http_requests/logging.h',
        'async_http_requests/async_http_requests/inc/async_http_requests/request_processor.h',
        'async_http_requests/async_http_requests/inc/async_http_requests/stack.h',
        'async_http_requests/async_http_requests/inc/async_http_requests/types.h',
        'async_http_requests/async_http_requests/src/external/inc/external/async_http_requests/ahr_curl.h',
        'async_http_requests/async_http_requests/src/private/inc/async_http_requests/private/ahr_mutex.h',
        'async_http_requests/async_http_requests/src/private/inc/async_http_requests/private/result.h',
    ],
    ext_modules=[
        Extension(
            'pyahr.libahr',
            [
                'async_http_requests/async_http_requests/src/async_http_requests.c',
                'async_http_requests/async_http_requests/src/logging.c',
                'async_http_requests/async_http_requests/src/request_processor.c',
                'async_http_requests/async_http_requests/src/stack.c',
                'async_http_requests/async_http_requests/src/external/src/ahr_curl.c',
                'async_http_requests/async_http_requests/src/private/src/ahr_mutex.c',
                'async_http_requests/async_http_requests/src/private/src/result.c',
            ],
            include_dirs=[
                'async_http_requests/async_http_requests/inc/',
                'async_http_requests/async_http_requests/src/external/inc/',
                'async_http_requests/async_http_requests/src/private/inc/',
            ],
            libraries=[
                'curl',
            ],
            extra_compile_args=[
                '-O2',
            ],
            language='C',
        )
    ],
)

