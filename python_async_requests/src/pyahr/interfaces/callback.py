"""Interface fuer einen Callback."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from abc import ABC, abstractmethod

#
# ---------------------------------------------------------------------------------------------------------------------
#

class Callback(ABC):
    
    @abstractmethod
    def on_success(self, response: Response) -> None:
        pass

    @abstractmethod
    def on_error(self, request: Request) -> None:
        pass
    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
