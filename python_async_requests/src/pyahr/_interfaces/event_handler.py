"""Interfacedefinition fuer einen Eventhandler."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from abc import ABC, abstractmethod

from .._transactions.response import AHR_Response

#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_EventHandler(ABC):
    """Interfacedefinition for an Eventhandler."""

    @abstractmethod
    def handle(self, response: AHR_Response) -> None:
        """This Function will be called for each received Response."""
        pass

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
