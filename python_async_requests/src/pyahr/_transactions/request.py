""""""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from typing_extensions import Self
from json import dumps
from typing import Dict, Optional, Any
from uuid import uuid4, UUID

#
# ---------------------------------------------------------------------------------------------------------------------
#

class Request:

    def __init__(self, ressource: str):
        self.__ressource: str = ressource
        self.__header = {}
        self.__parameter = {}
        pass
    
    def ressource(self) -> str:
        return self.__ressource

    def set_parameter(self, params: Dict[str, str]) -> Self:
        return self

    def set_header(self, header: Dict[str, str]) -> Self:
        return self

    def set_body(self, data: Optional[str]) -> Self:
        return self

    def body(self) -> Optional[str]:
        return None
    
    def to_repr(self) -> Any:
        return {
            'ressource': self.__ressource,
            'header': self.__header,
            'parameter': self.__parameter,
        }

    def __str__(self) -> str:
        return dumps(
            self.to_repr()
        )
    pass
#
# ---------------------------------------------------------------------------------------------------------------------
#
