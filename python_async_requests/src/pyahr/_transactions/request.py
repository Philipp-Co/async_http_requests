"""This Module implements a Request Object."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from typing_extensions import Self
from json import dumps
from typing import Dict, Optional, Any
from uuid import uuid4, UUID
from ctypes import c_size_t, c_void_p

#
# ---------------------------------------------------------------------------------------------------------------------
#

class Request:

    def __init__(self, obj: c_size_t):
        self.__ressource: str = ''
        self.__header = {}
        self.__parameter = {}
        self.__body = None
        self.__handle: c_size_t = obj
        pass

    def handle(self) -> c_size_t:
        return self.__handle
    
    def set_ressource(self, ressource: str) -> Self:
        self.__ressource = ressource
        return self

    def ressource(self) -> str:
        if len(self.__parameter) > 0:
            return self.__ressource + '?' + '&'.join([f'{parameter}={self.__parameter[parameter]}' for parameter in self.__parameter])
        return self.__ressource

    def set_query_parameter(self, parameter: Dict[str, str]) -> Self:
        self.__parameter = parameter
        return self

    def set_header(self, header: Dict[str, str]) -> Self:
        self.__header = header
        return self

    def header(self) -> Dict[str, str]:
        return self.__header

    def set_body(self, data: Optional[str]) -> Self:
        self.__body = data
        return self

    def query_parameter(self) -> Dict:
        return self.__parameter
    
    def body(self) -> Optional[str]:
        return self.__body

    def to_repr(self) -> Any:
        return {
            'handle': self.__handle,
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
