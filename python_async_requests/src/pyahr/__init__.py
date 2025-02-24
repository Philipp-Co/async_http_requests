"""Initialize the python_async_requests Module."""

_is_initialized: bool = False

if not _is_initialized:

    from ctypes import cdll, byref, POINTER,pointer, c_void_p, c_char, CFUNCTYPE, c_char_p, c_long, Structure, py_object, c_size_t
    from os import environ

    lib_path: str = environ.get('LIB_AHR_PATH', 'build/libahr.dylib')

    class AHR_HeaderEntry(Structure):

        _fields_ = [
            ('name', 256 * c_char),
            ('value', (4096 - 256) * c_char),
        ]
        
        pass

    class AHR_HeaderInfo(Structure):

        _fields_ = [
            ('header',  256 * AHR_HeaderEntry),
            ('nheader', c_size_t),
        ]


    _libahr = cdll.LoadLibrary(lib_path)

    _libahr.AHR_CreateLogger.argtypes = [
        py_object, 
        CFUNCTYPE(c_void_p, py_object, c_char_p),
        CFUNCTYPE(c_void_p, py_object, c_char_p),
        CFUNCTYPE(c_void_p, py_object, c_char_p),
    ]
    _libahr.AHR_CreateLogger.restype = POINTER(c_void_p)

    _libahr.AHR_ResponseBody.argtypes = [c_void_p]
    _libahr.AHR_ResponseBody.restype = c_char_p
            
    _libahr.AHR_ResponseStatusCode.argtypes = [c_void_p]
    _libahr.AHR_ResponseStatusCode.restype = c_long

    _libahr.AHR_ResponseHeader.argtypes = [c_void_p, c_void_p]
    # _libahr.AHR_ResponseHeader.restype = 

    _libahr.AHR_ResponseUUID.argtypes = [c_void_p]
    _libahr.AHR_ResponseUUID.restype = c_void_p


    _libahr.AHR_CreateProcessor.argtypes = [c_void_p]
    _libahr.AHR_CreateProcessor.restype = POINTER(c_void_p)


    class AHR_UserData(Structure):
        
        _fields_ = [
            ('data', py_object),
            ('on_success', CFUNCTYPE(c_void_p, py_object, c_void_p))
        ]
    _libahr.AHR_ProcessorGet.argtypes = [c_void_p, c_char_p, AHR_UserData]
    _libahr.AHR_ProcessorGet.restype = c_void_p

    _is_initialized = True
