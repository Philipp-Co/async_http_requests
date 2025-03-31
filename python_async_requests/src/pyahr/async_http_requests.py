"""This Module implements the HTTP Request Processor."""
#
# ---------------------------------------------------------------------------------------------------------------------
#

from copy import deepcopy
from ctypes import CFUNCTYPE, byref, c_char_p, c_size_t, c_void_p, py_object
from enum import IntEnum
from json import dumps
from logging import CRITICAL, DEBUG, ERROR, INFO, NOTSET, WARNING, Logger, getLogger
from typing import Dict, List, Optional, Tuple

from pyahr import AHR_Header, AHR_RequestData, AHR_UserData, _libahr
from typing_extensions import Self

from ._interfaces.event_handler import AHR_EventHandler
from ._transactions.request import AHR_Request, AHR_HttpMethod
from ._transactions.response import AHR_Response
from ._utils.string_decoder import AHR_IStringDecoder, AHR_Utf8Decoder

#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_ProcessorStatus(IntEnum):
    """Status Code."""

    AHR_PROC_OK = 0
    AHR_PROC_OBJECT_BUSY = 1
    AHR_PROC_UNKNOWN_OBJECT = 2
    AHR_PROC_NOT_ENOUGH_MEMORY = 3
    AHR_PROC_TIMEOUT = 4
    AHR_PROC_UNKNOWN_ERROR = 5

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_HttpProcessorFlowError(Exception):
    """A Flow Error.

    This Error is raised every time when the Flow of the Http Request Processor is disrupted.
    """

    def __init__(self, status: AHR_ProcessorStatus, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__status: AHR_ProcessorStatus = status
        pass

    def status(self) -> AHR_ProcessorStatus:
        return self.__status

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_DefaultEventHandler(AHR_EventHandler):
    """Defaulthandler to process AHR_Responses."""

    def __init__(self):
        super().__init__()
        self.__responses: List[AHR_Response] = []
        pass

    def handle(self, response: AHR_Response) -> None:
        self.__responses.append(response)
        pass

    def next(self) -> Optional[AHR_Response]:
        """Call this function to get the next AHR_Response."""
        if len(self.__responses) > 0:
            return self.__responses.pop(0)
        return None

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#


class AHR_HttpRequestProcessor:
    """HTTP Request Processor.

    Example:
        event_handler: AHR_DefaultEventHandler = AHR_DefaultEventHandler()
        p: AHR_HttpRequestProcessor = AHR_HttpRequestProcessor(
            url='http://www.google.de',
            event_handler=event_handler,
        )

        #
        # Request "http://www.google.de/some/ressource/?foor=bar"
        #

        # Create and prepare a Request.
        request: AHR_Request = p.request().set_ressource(
            'some/ressource/'
        ).set_header(
            {'test': 'yes'}
        ).set_query_parameter(
            {'foo', 'bar'}
        )
        # Set HTTP Method for Request and make the Request.
        p.get(
            request
        ).make_request(
            request
        )

        ...

        while True:
            response: Optional[AHR_Response] = event_handler.next()
            if response is not None:
                print(response)
    """

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

    def __init__(
        self,
        url: str,
        event_handler: AHR_EventHandler,
        max_number_of_requestobjects: int = 5,
        logger: Optional[Logger] = None,
    ):
        """Constructor.

        Args:
            url: str: The Base URL to send Requests to.
            event_handler: AHR_EventHandler: A Handler that is called for each received HTTP Response.
            max_number_of_requestobjects: int = 5: Number of available Requestobjects, 1 <= x <= 25.
            logger: Optional[Logger] = None: The Logger to be used.
        """
        # Python Logger.
        self.__logger: Logger = logger if logger is not None else getLogger(self.__class__.__name__)
        # Configure libahr Logging.
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
        # HttpProcessor Handle from libahr.
        max_number_of_requestobjects = max(min(max_number_of_requestobjects, 25), 1)
        self.__ahr_processor: c_void_p = _libahr.AHR_CreateProcessor(max_number_of_requestobjects, self.__ahr_logger)
        if self.__ahr_processor is None:
            raise AssertionError(
                'Unable to create Instance with given Arguments - '
                f'max_number_of_requestobjects={max_number_of_requestobjects}.'
            )

        # Response Body Decoder
        self.__string_decoder: AHR_IStringDecoder = AHR_Utf8Decoder()

        # URL this Instance uses.
        self.__url: str = url[0 : len(url) - 1] if url.endswith('/') else url  # noqa: E203

        # Eventhandler wich is called for each Response to a Request.
        self.__event_handler: AHR_EventHandler = event_handler

        # Table for known Request/Response Objects and
        # and for Copies of ongoing requests.
        self.__requests: Dict[int, AHR_Request] = {}
        self.__request_objects: Dict[int, Tuple[bool, AHR_Request]] = {}
        for i in range(0, _libahr.AHR_ProcessorNumberOfRequestObjects(self.__ahr_processor)):
            self.__request_objects[i] = (False, AHR_Request(i))

        # Start this Instance.
        if not _libahr.AHR_ProcessorStart(self.__ahr_processor):
            raise RuntimeError('Unable to start Http Request Processor!')
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
        _libahr.AHR_ProcessorStop(self.__ahr_processor)
        _libahr.AHR_DestroyProcessor(byref(self.__ahr_processor))
        _libahr.AHR_DestroyLogger(byref(self.__ahr_logger))
        pass

    def set_string_decoder(self, decoder: AHR_IStringDecoder) -> Self:
        """Set the Stringdecode.

        The Stringdecoder is used to decode the Responsebody received by the remote server.

        Returns:
            Self: A Reference to this object.
        """
        self.__string_decoder = decoder
        return self

    @CFUNCTYPE(c_void_p, py_object, c_size_t, c_size_t)
    def __on_error(self, robject, error_code) -> None:
        """This Function is called by libahr in case Request has failed with an Error."""
        try:
            self.__logger.info('Error: Response received...')
            self.__logger.info(
                f'An Error occured during handling of Requestobject {robject}, Error Code ist {error_code}.'
            )
            self.__event_handler.handle(
                AHR_Response(self.__requests[robject]).set_error_code(error_code).set_status_code(500).set_body('')
            )
            self.__requests.pop(robject)
        except Exception as e:  # noqa: B902
            if robject in self.__requests:
                self.__requests.pop(robject)

            self.__logger.exception(e)
            self.__event_handler.handle(AHR_Response(self.__requests[robject]).set_status_code(500)).set_body('')
        pass

    @CFUNCTYPE(c_void_p, py_object, c_size_t, c_size_t, c_char_p, c_size_t)
    def __on_success(self, robject: c_size_t, status_code: c_size_t, buffer: c_char_p, nbytes: c_size_t) -> None:
        """This function is called from libahr when the response to a request was fully received.

        Args:
            robject: py_object: A Pythonobject.
            status_code: c_size_t: A Status Code, 0 <= x.
            buffer: c_char_p: Buffer with "nbytes" length.
            nbytes: Number of Bytes in "buffer".
        """
        try:
            self.__logger.info('Success: Response received...')
            self.__logger.info(
                self.__requests
            )
            self.__event_handler.handle(
                AHR_Response(self.__requests[robject])
                .set_status_code(status_code)
                .set_body(self.__string_decoder.decode(buffer if buffer is not None else ''))
            )
            self.__requests.pop(robject)
        except Exception as e:  # noqa: B902
            if robject in self.__requests:
                self.__requests.pop(robject)

            self.__logger.exception(e)
            #self.__event_handler.handle(AHR_Response(self.__requests[robject]).set_status_code(-1))
        pass

    def __create_request_data(self, request: AHR_Request) -> AHR_RequestData:
        """Create the AHR_RequestData Structure and fill it with the Requests contents."""
        request_data = AHR_RequestData()
        request_data.header = AHR_Header()
        request_data.nheader = 0

        url: str = f'{self.__url}/{request.ressource()}\0'
        request_data.url = url.encode()
        request_data.body = request.body().encode() if request.body() is not None else None

        i = 0
        for header in request.header():
            request_data.header.header[i].name = header.encode()
            request_data.header.header[i].value = request.header()[header].encode()
            i = i + 1
        request_data.nheader = i
        return request_data

    def create_request(self) -> AHR_Request:
        """Get a Requestobject.

        Raises:
            MemoryError: If there are no more Objects available.
        """
        item: int
        for item in self.__request_objects:
            if not self.__request_objects[item][0]:
                self.__request_objects[item] = (True, self.__request_objects[item][1])
                return self.__request_objects[item][1]
        raise MemoryError('Currently no more Requestobjects available.')

    def make_request(self, request: AHR_Request) -> Self:
        """Make a Request.

        Raises:
            AHR_HttpProcessorFlowError: If the Request Object is currently busy or if an Error occured.
        """
        if request in self.__requests:
            raise AHR_HttpProcessorFlowError(status=AHR_ProcessorStatus.AHR_PROC_OBJECT_BUSY)

        self.__requests[request.handle()] = request # deepcopy(request)
        self.__logger.info(
            f'Make Request wiht handle {request.handle()}'
        )
        self.__logger.info(
            self.__requests
        )
        res: AHR_ProcessorStatus = AHR_ProcessorStatus(
            _libahr.AHR_ProcessorMakeRequest(self.__ahr_processor, request.handle())
        )
        if AHR_ProcessorStatus.AHR_PROC_OK != res:
            raise AHR_HttpProcessorFlowError(status=res)
        return self

    def configure_request(self, request: AHR_Request) -> Self:
        """Configure a Request Object.
        
        The internal C-Structures are updated. Call this function whenever the Python Request Object changes.
        """
        request_data: AHR_RequestData = self.__create_request_data(request)
        status: AHR_ProcessorStatus
        match request.http_method():
            case AHR_HttpMethod.GET:
                status = AHR_ProcessorStatus(
                    _libahr.AHR_ProcessorGet(
                        self.__ahr_processor,
                        request.handle(),
                        byref(request_data),
                        AHR_UserData(
                            py_object(self),
                            self.__on_success,
                            self.__on_error,
                        ),
                    )
                )
                pass
            case AHR_HttpMethod.POST:
                print('Configure POST')
                status = AHR_ProcessorStatus(
                    _libahr.AHR_ProcessorPost(
                        self.__ahr_processor,
                        request.handle(),
                        byref(request_data),
                        AHR_UserData(
                            py_object(self),
                            self.__on_success,
                            self.__on_error,
                        ),
                    )
                )
            case AHR_HttpMethod.PUT:
                status = AHR_ProcessorStatus(
                    _libahr.AHR_ProcessorPut(
                        self.__ahr_processor,
                        request.handle(),
                        byref(request_data),
                        AHR_UserData(
                            py_object(self),
                            self.__on_success,
                            self.__on_error,
                        ),
                    )
                )
            case AHR_HttpMethod.DELETE:
                status = AHR_ProcessorStatus(
                    _libahr.AHR_ProcessorDelete(
                        self.__ahr_processor,
                        request.handle(),
                        byref(request_data),
                        AHR_UserData(
                            py_object(self),
                            self.__on_success,
                            self.__on_error,
                        ),
                    )
                )
            case _:
                raise AssertionError(f'Unknown HTTP Method: {request.http_method()}.')

        if AHR_ProcessorStatus.AHR_PROC_OK != status:
            raise AHR_HttpProcessorFlowError(status=status)
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
        return list(self.__requests.keys())

    def __repr__(self):
        """Returns a Representation as Python Nativ Structures."""
        return {
            'pending_requests': list(self.__requests.keys()),
        }

    def __str__(self) -> str:
        """To String Function."""
        return dumps(self.__repr__())

    pass


#
# ---------------------------------------------------------------------------------------------------------------------
#
