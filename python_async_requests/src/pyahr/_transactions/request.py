"""This Module implements a Request Object."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from ctypes import c_size_t
from json import dumps
from typing import Any, Dict, Optional

from typing_extensions import Self
from enum import IntEnum

#
# ---------------------------------------------------------------------------------------------------------------------
#

class AHR_HttpMethod(IntEnum):

    GET = 0
    POST = 1
    PUT = 2
    DELETE = 3

    pass
#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_Request:
    """AHR Requestobject."""

    def __init__(self, obj: c_size_t):
        self.__ressource: str = ''
        self.__header = {}
        self.__parameter = {}
        self.__body = None
        self.__handle: c_size_t = obj
        self.__http_method: AHR_HttpMethod = AHR_HttpMethod.GET
        pass

    def handle(self) -> c_size_t:
        return self.__handle

    def set_ressource(self, ressource: str) -> Self:
        self.__ressource = ressource
        return self

    def ressource(self) -> str:
        if len(self.__parameter) > 0:
            return (
                self.__ressource
                + '?'  # noqa: W503
                + '&'.join(  # noqa: W503
                    [f'{parameter}={self.__parameter[parameter]}' for parameter in self.__parameter]  # noqa: W503
                )
            )
        return self.__ressource
    
    def http_method(self) -> str:
        return self.__http_method
    
    def set_http_method(self, http_method: AHR_HttpMethod) -> Self:
        self.__http_method = http_method
        return self

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
        """Converts this Object to nativ Python Structures."""
        return {
            'http_method': self.__http_method,
            'handle': self.__handle,
            'ressource': self.__ressource,
            'header': self.__header,
            'parameter': self.__parameter,
        }

    def __deepcopy__(self, el):
        raise AssertionError('This Instance can not be Deep-Copied!')
    
    def __copy__(self):
        raise AssertionError('This Instance can not be Copied!')

    def __str__(self) -> str:
        """To String Function."""
        return dumps(self.to_repr())

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
