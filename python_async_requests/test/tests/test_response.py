from pyahr._transactions.request import Request
from pyahr._transactions.response import Response
from unittest import TestCase


class Initial(TestCase):

    def runTest(self):

        response: Response = Response(
            request=Request(ressource='')
        )

        pass

    pass

