"""Handler for decoding Strings."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from pyahr._interfaces.string_decoder import AHR_IStringDecoder

#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_Utf8Decoder(AHR_IStringDecoder):
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
