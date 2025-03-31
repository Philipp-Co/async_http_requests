"""This Module implements a Response Object."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from json import dumps
from typing import Any, Dict, Optional

from enum import IntEnum
from typing_extensions import Self

from .request import AHR_Request

#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_ErrorCode(IntEnum):
    """Error Code."""

    CIRICAL_ERROR = 0

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_Response:
    """A AHR Response Object."""

    def __init__(self, request: AHR_Request):
        self.__body: Optional[str] = None
        self.__status_code: int = 500
        self.__header: Dict[str, str] = {}
        self.__request: AHR_Request = request
        self.__error_code: Optional[AHR_ErrorCode] = None
        pass

    def set_error_code(self, value: AHR_ErrorCode) -> Self:
        self.__error_code = value
        return self

    def error_code(self) -> Optional[AHR_ErrorCode]:
        return self.__error_code

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
        if not self.is_valid():
            raise IOError('Invalid Object.')
        return self.__status_code

    def body(self) -> Optional[str]:
        if not self.is_valid():
            raise IOError('Invalid Object.')
        return self.__body

    def header(self) -> Dict[str, str]:
        if not self.is_valid():
            raise IOError('Invalid Object.')
        return self.__header

    def request(self) -> AHR_Request:
        return self.__request

    def to_repr(self) -> Any:
        return {
            'request': self.__request.to_repr(),
            'header': self.__header,
            'status_code': self.__status_code,
            'error_code': self.__error_code,
            'body': (self.__body if len(self.__body) <= 32 else f'{self.__body[0:29]}...')
            if self.__body is not None
            else None,
        }

    def is_valid(self) -> bool:
        return self.error_code() is None

    def __bool__(self) -> bool:
        """This Object is valid if the boolean Conversion evaluates to True."""
        return self.is_valid()

    def __str__(self) -> str:
        """To String Function."""
        return dumps(self.to_repr())

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
