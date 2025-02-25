
#
# ---------------------------------------------------------------------------------------------------------------------
#

from pyahr.async_http_requests import Processor, DefaultEventHandler
from unittest import TestCase

#
# ---------------------------------------------------------------------------------------------------------------------
#

class Initial(TestCase):

    def runTest(self):

        event_handler: DefaultEventHandler = DefaultEventHandler()
        processor: Processor = Processor(url='www.google.de', event_handler=event_handler)

        pass

    pass

#
# ---------------------------------------------------------------------------------------------------------------------
#
