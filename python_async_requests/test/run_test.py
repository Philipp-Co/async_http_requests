#!python

from unittest import TestCase, TestSuite, TextTestRunner, TestLoader


runner: TextTestRunner = TextTestRunner()
runner.run(
    TestLoader().discover(
        start_dir='tests/',
        pattern='test_*.py',
    )
)

