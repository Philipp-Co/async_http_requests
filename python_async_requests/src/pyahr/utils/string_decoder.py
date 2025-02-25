"""Handler for decoding Strings."""
#
# ---------------------------------------------------------------------------------------------------------------------
#
from abc import ABC, abstractmethod

#
# ---------------------------------------------------------------------------------------------------------------------
#
class StringDecoder(ABC):
    """Interface."""
    @abstractmethod
    def decode(self, data: bytes) -> str:
        """Decode String."""
        pass

    pass
#
# ---------------------------------------------------------------------------------------------------------------------
#

class Utf8Decoder(StringDecoder):
    """Konkreter Decoder fuer UTF-8.
        
    Decode to UTF-8 and replace unknown characters.
    """
    def __init__(self):
        super().__init__()
        pass

    def decode(self, data: bytes) -> str:
        return data.decode('utf-8', 'replace')

    pass
#
# ---------------------------------------------------------------------------------------------------------------------
#
