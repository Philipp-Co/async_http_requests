"""Initialize the python_async_requests Module."""

#
# ---------------------------------------------------------------------------------------------------------------------
#

_is_initialized: bool = False

if not _is_initialized:

    from ctypes import cdll, byref, POINTER,pointer, c_void_p, c_char, CFUNCTYPE, c_char_p, c_long, Structure, py_object, c_size_t, c_bool, c_int
    from os import environ
    
    lib_path: str = environ.get('LIB_AHR_PATH', 'libahr.dylib')

    class AHR_HeaderEntry(Structure):

        _fields_ = [
            ('name', 256 * c_char),
            ('value', (4096 - 256) * c_char),
        ]
        
        pass

    class AHR_Header(Structure):

        _fields_ = [
            ('header',  256 * AHR_HeaderEntry),
            ('nheader', c_size_t),
        ]


    _libahr = cdll.LoadLibrary(lib_path)
    
    #
    # =====================================================
    #
    _libahr.AHR_CreateLogger.argtypes = [
        py_object, 
        CFUNCTYPE(c_void_p, py_object, c_char_p),
        CFUNCTYPE(c_void_p, py_object, c_char_p),
        CFUNCTYPE(c_void_p, py_object, c_char_p),
    ]
    _libahr.AHR_CreateLogger.restype = POINTER(c_void_p)

    _libahr.AHR_LoggerSetLoglevel.argtyps = [c_void_p, c_size_t]
    _libahr.AHR_LoggerSetLoglevel.restype = c_void_p
    #
    # =====================================================
    #
    _libahr.AHR_ResponseBody.argtypes = [c_void_p]
    _libahr.AHR_ResponseBody.restype = c_char_p
    #
    # =====================================================
    #
    _libahr.AHR_ResponseStatusCode.argtypes = [c_void_p]
    _libahr.AHR_ResponseStatusCode.restype = c_long
    #
    # =====================================================
    #
    _libahr.AHR_ResponseHeader.argtypes = [c_void_p, c_void_p]
    # _libahr.AHR_ResponseHeader.restype = 
    #
    # =====================================================
    #
    _libahr.AHR_ResponseUUID.argtypes = [c_void_p]
    _libahr.AHR_ResponseUUID.restype = c_void_p
    #
    # =====================================================
    #
    _libahr.AHR_CreateProcessor.argtypes = [c_size_t, c_void_p]
    _libahr.AHR_CreateProcessor.restype = POINTER(c_void_p)

    _libahr.AHR_ProcessorStart.argtypes = [c_void_p]
    _libahr.AHR_ProcessorStart.restype = c_bool 

    _libahr.AHR_ProcessorStop.argtypes = [c_void_p]
    # _libahr.AHR:ProcessorStop.restype = 

    #
    # =====================================================
    #
    class AHR_UserData(Structure):
        
        _fields_ = [
            ('data', py_object),
            ('on_success', CFUNCTYPE(c_void_p, py_object, c_size_t, c_size_t, c_char_p, c_size_t)),
            ('on_error', CFUNCTYPE(c_void_p, py_object, c_size_t, c_size_t)),
        ]
    
    class AHR_RequestData(Structure):

        _fields_ = [
            ('header', AHR_Header),
            ('url', c_char_p),
            ('body', c_char_p),
            ('log_level', c_size_t),
        ]
    #
    # =====================================================
    #
   
    _libahr.AHR_ProcessorTransactionId.argtyps = [c_void_p, c_size_t]
    _libahr.AHR_ProcessorTransactionId.restype = c_void_p

    _libahr.AHR_ProcessorNumberOfRequestObjects.argtypes = [c_void_p]
    _libahr.AHR_ProcessorNumberOfRequestObjects.restype = c_size_t

    #
    # =====================================================
    #
    _libahr.AHR_ProcessorMakeRequest.argtypes = [c_void_p, c_size_t]
    _libahr.AHR_ProcessorMakeRequest.restype = c_int

    _libahr.AHR_ProcessorGet.argtypes = [
        c_void_p, 
        c_size_t,
        POINTER(AHR_RequestData),
        AHR_UserData,
    ]
    _libahr.AHR_ProcessorGet.restype = c_int
    
    _libahr.AHR_ProcessorPost.argtypes = [
        c_void_p, 
        c_size_t,
        POINTER(AHR_RequestData),
        AHR_UserData,
    ]
    _libahr.AHR_ProcessorPost.restype = c_int
    
    _libahr.AHR_ProcessorPut.argtypes = [
        c_void_p, 
        c_size_t,
        POINTER(AHR_RequestData),
        AHR_UserData,
    ]
    _libahr.AHR_ProcessorPut.restype = c_int
    
    _libahr.AHR_ProcessorDelete.argtypes = [
        c_void_p, 
        c_size_t,
        POINTER(AHR_RequestData),
        AHR_UserData,
    ]
    _libahr.AHR_ProcessorDelete.restype = c_int
    #
    # =====================================================
    #

    _is_initialized = True
#
# ---------------------------------------------------------------------------------------------------------------------
#

