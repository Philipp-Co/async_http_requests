""""""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from ._transactions.request import Request
from ._transactions.response import Response
from typing_extensions import Self
from abc import ABC, abstractmethod
from logging import getLogger, Logger
from typing import Optional, List
from ctypes import cdll, byref, POINTER,pointer, c_void_p, c_char, CFUNCTYPE, c_char_p, Structure, py_object
from ctypes import c_size_t
from python_async_requests.pyahr import _libahr, AHR_UserData, AHR_HeaderInfo, AHR_HeaderEntry
from .utils.string_decoder import StringDecoder, Utf8Decoder
from copy import deepcopy
from .interfaces.event_handler import EventHandler

#
# ---------------------------------------------------------------------------------------------------------------------
#

class DefaultEventHandler(EventHandler):
    
    def __init__(self):
        super().__init__()
        self.__responses: List[Response] = []
        pass

    def handle(self, response: Response) -> None:
        self.__responses.append(
            response
        )
        pass

    def next(self) -> Optional[Response]:
        if len(self.__responses) > 0:
            return self.__responses.pop(0)
        return None

    pass

#
# ---------------------------------------------------------------------------------------------------------------------
#


class Processor:

    def __init__(self, url: str, event_handler: EventHandler, logger: Optional[Logger] = None):
        self.__logger: Logger = logger if logger is not None else getLogger(self.__class__.__name__);
        # AHR_Processotr_t Handle
        self.__ahr_processor = _libahr.AHR_CreateProcessor()
        # Response Body Decoder
        self.__string_decoder: StringDecoder = Utf8Decoder()

        self.__url: str = url[0:len(url)-1] if url.endswith('/') else url
        self.__requests = {}
        self.__event_handler: EventHandler = event_handler
        pass

    def __reduce__ex(self):
        raise AssertionError('This object may not be serialized due to dependencies to libahr.')

    def __reduce__(self):
        raise AssertionError('This object may not be serialized due to dependencies to libahr.')

    def __del__(self):
        _libahr.AHR_DestroyProcessor(byref(self.__ahr_processor))
        pass

    def set_string_decoder(self, decoder: StringDecoder) -> Self:
        self.__string_decoder = decoder
        return self

    @CFUNCTYPE(c_void_p, py_object, c_void_p)
    def __callback(self, response) -> None:
        """This function is called from libahr when the response to a request was fully received.

        Args:
            response:   This is a ctype Object for AHR_Response_t Object in libahr. 
                        This Object can be used as an Argument for calls to
                        "AHR_Response*". 
        """
        response_id = _libahr.AHR_ResponseUUID(response)
        try:
            info = AHR_HeaderInfo()
            _libahr.AHR_ResponseHeader(response, byref(info))
            self.__event_handler.handle(
                Response(
                    self.__requests[response_id]
                ).set_status_code(
                    _libahr.AHR_ResponseStatusCode(response)
                ).set_header(
                    {
                        info.header[i].name.decode().rstrip(): info.header[i].value.decode().rstrip() for i in range(0, info.nheader)
                    } 
                ).set_body(
                    self.__string_decoder.decode(
                        _libahr.AHR_ResponseBody(response)
                    )
                )
            )
            self.__requests.pop(response_id)
        except Exception as e:
            self.__logger.exception(e)
            self.__event_handler.handle(
                Response(
                    self.__requests[response_id]
                ).set_status_code(
                    -1
                )
            )
        pass

    def get(self, request: Request) -> Self:
        #
        # deepcopy the request object.
        # the requestobject shall not be changed during the request-process.
        #
        request_id = _libahr.AHR_ProcessorGet(
            self.__ahr_processor,
            f'{self.__url}/{request.ressource()}'.encode(),
            AHR_UserData(
                py_object(self),
                self.__callback,
            )
        )
        self.__requests[request_id] = deepcopy(request)
        return self
    
    def post(self, equest: Request) -> Self:
        return self

    def put(self, request: Request) -> Self:
        return self

    def delete(self, request: Request) -> Self:
        return self
    
    def number_of_pending_requests(self) -> int:
        return len(self.__requests)

    def pending_requests(self) -> List[int]:
        return [key for key in self.__requests]

    def __repr__(self):
        return {
            'pending_requests': [
                key for key in self.__requests.keys()
            ], 
        }

    pass

#
# ---------------------------------------------------------------------------------------------------------------------
#
