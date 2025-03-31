from abc import ABC, abstractmethod


#
# ---------------------------------------------------------------------------------------------------------------------
#
class AHR_IStringDecoder(ABC):
    """Interface."""

    @abstractmethod
    def decode(self, data: bytes) -> str:
        """Decode String."""
        pass

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
