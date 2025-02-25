"""This Module implements a Request Object."""
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
        self.__body = None
        pass
    
    def ressource(self) -> str:
        return self.__ressource

    def set_header(self, header: Dict[str, str]) -> Self:
        self.__header = header
        return self

    def header(self) -> Dict[str, str]:
        return self.__header

    def set_body(self, data: Optional[str]) -> Self:
        self.__body = data
        return self

    def set_parameter(self, data: Dict[str, str]) -> Self:
        self.__paramater = data
        return self

    def parameter(self) -> Dict:
        return self.__parameter
    
    def body(self) -> Optional[str]:
        return self.__body
    
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
