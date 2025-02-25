#!python
"""Unittest for python_async_http_requests."""

#
# ---------------------------------------------------------------------------------------------------------------------
#

if __name__ == '__main__':

    from unittest import TestCase, TestSuite, TextTestRunner, TestLoader

    runner: TextTestRunner = TextTestRunner()
    exit(
        1-int(
            runner.run(
                TestLoader().discover(
                    start_dir='tests/',
                    pattern='test_*.py',
                )
            ).wasSuccessful()
        )
    )

exit(-1)

#
# ---------------------------------------------------------------------------------------------------------------------
#
