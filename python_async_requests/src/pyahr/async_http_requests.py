"""This Module implements the HTTP Request Processor."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from ._transactions.request import Request
from ._transactions.response import Response
from typing_extensions import Self
from abc import ABC, abstractmethod
from logging import getLogger, Logger
from typing import Optional, List
from ctypes import cdll, byref, POINTER,pointer, c_void_p, c_char, CFUNCTYPE, c_char_p, Structure, py_object, byref
from ctypes import c_size_t
from pyahr import _libahr, AHR_UserData, AHR_Header, AHR_HeaderEntry, AHR_RequestData 
from .utils.string_decoder import StringDecoder, Utf8Decoder
from copy import deepcopy
from .interfaces.event_handler import EventHandler
from json import dumps
from logging import NOTSET, DEBUG, INFO, WARNING, ERROR, CRITICAL

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
    """HTTP Request Processor."""
    
    @CFUNCTYPE(c_void_p, py_object, c_char_p)
    def __log_info(self, msg):
        self.__logger.info(msg)
        pass
    
    @CFUNCTYPE(c_void_p, py_object, c_char_p)
    def __log_warning(self, msg):
        self.__logger.warning(msg)
        pass
    
    @CFUNCTYPE(c_void_p, py_object, c_char_p)
    def __log_error(self, msg):
        self.__logger.error(msg)
        pass

    def __map_loglevel(self, loglevel: int) -> int:
        """Convert the logging Modules Loglevel to a loglevel that libahr can understand."""
        try:
            return {
                NOTSET: 0,
                DEBUG: 0,
                INFO: 0,
                WARNING: 1,
                ERROR: 2,
                CRITICAL: 2,
            }[loglevel]
        except KeyError:
            return 2
    
    def __init__(self, url: str, event_handler: EventHandler, logger: Optional[Logger] = None):
        """Constructor.
        
        Args:
            url: str: The Base URL to send Requests to.
            event_handler: EventHandler: A Handler that is called for each received HTTP Response.
            logger: Optional[Logger] = None: The Logger to be used.
        """
        self.__logger: Logger = logger if logger is not None else getLogger(self.__class__.__name__);
        # AHR_Processotr_t Handle
        func = CFUNCTYPE(c_void_p, c_void_p, c_char_p),
        self.__ahr_logger = _libahr.AHR_CreateLogger(
            py_object(self),
            self.__log_info,
            self.__log_warning,
            self.__log_error,
        )
        _libahr.AHR_LoggerSetLoglevel(
            self.__ahr_logger,
            c_size_t(self.__map_loglevel(self.__logger.level)),
        )

        self.__ahr_processor = _libahr.AHR_CreateProcessor(
            self.__ahr_logger
        )
        # Response Body Decoder
        self.__string_decoder: StringDecoder = Utf8Decoder()

        self.__url: str = url[0:len(url)-1] if url.endswith('/') else url
        self.__requests = {}
        self.__event_handler: EventHandler = event_handler
        pass

    def __reduce__ex(self):
        """Due to libahr Dependencies reduction is not allowed."""
        raise AssertionError('This object may not be serialized due to dependencies to libahr.')

    def __reduce__(self):
        """Due to libahr Dependencies reduction is not allowed."""
        raise AssertionError('This object may not be serialized due to dependencies to libahr.')

    def __del__(self):
        """Destructor.
        
        Destroyes created _libahr Objects.
        """
        _libahr.AHR_DestroyProcessor(byref(self.__ahr_processor))
        _libahr.AHR_DestroyLogger(byref(self.__ahr_logger))
        pass

    def set_string_decoder(self, decoder: StringDecoder) -> Self:
        """Set the Stringdecode.
        
        The Stringdecoder is used to decode the Responsebody received by the remote server.

        Returns:
            Self: A Reference to this object.
        """
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
            info = AHR_Header()
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
    
    def __create_request_data(self, request: Request) -> AHR_RequestData:
        request_data = AHR_RequestData()
        request_data.header = AHR_Header()
        request_data.nheader = 0
        request_data.url = f'{self.__url}/{request.ressource()}'.encode()
        request_data.body = None
        
        i = 0
        for header in request.header():
            request_data.header.header[i].name = header.encode()
            request_data.header.header[i].value = request.header()[header].encode()
            i = i + 1
        request_data.nheader = i
        return request_data
    

    def get(self, request: Request) -> Self:
        """Create and execute a HTTP GET Request.

        It is not allowed to append a Payload in the Requet Body. 

        Example:
            p.get(
                Request(ressource='')
            )
        """
        #
        # deepcopy the request object.
        # the requestobject shall not be changed during the request-process.
        #
        request_data: AHR_RequestData = self.__create_request_data(request)
        request_id = _libahr.AHR_ProcessorGet(
            self.__ahr_processor,
            byref(request_data),
            AHR_UserData(
                py_object(self),
                self.__callback,
            )
        )
        self.__requests[request_id] = deepcopy(request)
        return self
    
    def post(self, equest: Request) -> Self:
        """Create and execute a HTTP POST Request.
        
        Post Requets are always done as Application/JSON with a payload in the Request Body. 
        
        Example:
            p.post(
                Request(
                    ressource=''
                ).set_body(
                    dumps(
                        {
                            'foo': 'bar',
                        }
                    )
                )
            ) 
        """
        request_data: AHR_RequestData = self.__create_request_data(request)
        request_id = _libahr.AHR_ProcessorPost(
            self.__ahr_processor,
            byref(request_data),
            AHR_UserData(
                py_object(self),
                self.__callback,
            )
        )
        self.__requests[request_id] = deepcopy(request)
        return self

    def put(self, request: Request) -> Self:
        """Create and execute a HTTP PUT Request.

        Example:
            See post().
        """
        request_data: AHR_RequestData = self.__create_request_data(request)
        request_id = _libahr.AHR_ProcessorPut(
            self.__ahr_processor,
            byref(request_data),
            AHR_UserData(
                py_object(self),
                self.__callback,
            )
        )
        self.__requests[request_id] = deepcopy(request)
        return self

    def delete(self, request: Request) -> Self:
        """Create and execute a HTTP DELETE Request.
        
        Example:
            See get().
        """
        request_data: AHR_RequestData = self.__create_request_data(request)
        request_id = _libahr.AHR_ProcessorDelete(
            self.__ahr_processor,
            byref(request_data),
            AHR_UserData(
                py_object(self),
                self.__callback,
            )
        )
        self.__requests[request_id] = deepcopy(request)
        return self
    
    def number_of_pending_requests(self) -> int:
        """Get the number of pending requests.
        
        Returns the number of requests for which no response was received yet.

        Returns:
            int: A Number.
        """
        return len(self.__requests)

    def pending_requests(self) -> List[int]:
        """Returns the Ids of pending requests."""
        return [key for key in self.__requests]

    def __repr__(self):
        return {
            'pending_requests': [
                key for key in self.__requests.keys()
            ], 
        }
    
    def __str__(self) -> str:
        return dumps(
            self.__repr__()
        )

    pass

#
# ---------------------------------------------------------------------------------------------------------------------
#
