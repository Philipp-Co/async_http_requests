"""Interfacedefinition fuer einen Eventhandler."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from abc import ABC, abstractmethod
from .._transactions.response import Response

#
# ---------------------------------------------------------------------------------------------------------------------
#

class EventHandler(ABC):
   
    @abstractmethod
    def handle(self, response: Response) -> None:
        pass

    pass

#
# ---------------------------------------------------------------------------------------------------------------------
#
