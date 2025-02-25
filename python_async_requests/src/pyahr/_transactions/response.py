"""This Module implements a Response Object."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from typing_extensions import Self
from json import dumps
from typing import Dict, Optional, Any
from .request import Request

#
# ---------------------------------------------------------------------------------------------------------------------
#

class Response:
    
    def __init__(self, request: Request):
        self.__body: Optional[str] = None
        self.__status_code: int = 500
        self.__header: Dict[str, str] = {}
        self.__request: Request = request
        pass

    def set_header(self, header: Dict[str, str]) -> Self:
        self.__header = header
        return self

    def set_body(self, data: Optional[str]) -> Self:
        self.__body = data
        return self

    def set_status_code(self, code: int) -> Self:
        self.__status_code = code
        return self

    def status_code(self) -> int:
        return self.__status_code

    def body(self) -> Optional[str]:
        return self.__body
    
    def header(self) -> Dict[str, str]:
        return self.__header

    def request(self) -> Request:
        return self.__request

    def to_repr(self) -> Any:
        return {
            'request': self.__request.to_repr(),
            'header': self.__header,
            'status_code': self.__status_code,
            'body': (self.__body if len(self.__body) <= 32 else f'{self.__body[0:29]}...') if self.__body is not None else None,
        }

    def __str__(self) -> str:
        return dumps(
            self.to_repr()
        )

    def valid(self) -> bool:
        return self.status_code() >= 0

    def __bool__(self) -> bool:
        return self.valid()

    pass
#
# ---------------------------------------------------------------------------------------------------------------------
#
