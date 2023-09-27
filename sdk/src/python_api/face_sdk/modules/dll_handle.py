from ctypes import CDLL
from ctypes import c_void_p, c_char_p, POINTER
from ctypes import c_uint32, c_double, c_ulong, c_long, c_bool


class DllHandle:
    __contextNamespace = 'TDVContext_'
    __exceptionNamcepsace = 'TDVException_'

    def __init__(self, dll_handle: CDLL):
        self.__dll_handle = dll_handle

    def apiException_what(self, *args, **kwargs):
        func = self.__dll_handle['{}getMessage'.format(self.__exceptionNamcepsace)]
        func.restype = c_void_p

        return func(*args, **kwargs)

    def apiException_code(self, *args, **kwargs):
        func = self.__dll_handle['{}getErrorCode'.format(self.__exceptionNamcepsace)]
        func.restype = c_uint32

        return func(*args, **kwargs)

    def apiObject_destructor(self, *args, **kwargs):
        self.__dll_handle['{}deleteException'.format(self.__exceptionNamcepsace)](*args, **kwargs)


    def create(self, *args, **kwargs):
        func = self.__dll_handle['{}create'.format(self.__contextNamespace)]
        func.restype = c_void_p

        return func(*args, **kwargs)

    def destroy(self, *args, **kwargs):
        self.__dll_handle['{}destroy'.format(self.__contextNamespace)](*args, **kwargs)

    def freePtr(self, *args, **kwargs):
        self.__dll_handle['{}freePtr'.format(self.__contextNamespace)](*args, **kwargs)


    def copy(self, *args, **kwargs):
        self.__dll_handle['{}copy'.format(self.__contextNamespace)](*args, **kwargs)

    def clone(self, *args, **kwargs):
        func = self.__dll_handle['{}clone'.format(self.__contextNamespace)]
        func.restype = c_void_p
        return func(*args, **kwargs)


    def putStr(self, *args, **kwargs):
        self.__dll_handle['{}putStr'.format(self.__contextNamespace)](*args, **kwargs)

    def putDouble(self, *args, **kwargs):
        self.__dll_handle['{}putDouble'.format(self.__contextNamespace)](*args, **kwargs)

    def putLong(self, *args, **kwargs):
        self.__dll_handle['{}putLong'.format(self.__contextNamespace)](*args, **kwargs)

    def putBool(self, *args, **kwargs):
        self.__dll_handle['{}putBool'.format(self.__contextNamespace)](*args, **kwargs)

    def putDataPtr(self, *args, **kwargs):
        func = self.__dll_handle['{}putDataPtr'.format(self.__contextNamespace)]
        func.restype = c_char_p
        return func(*args, **kwargs)

    def pushBack(self, *args, **kwargs):
        self.__dll_handle['{}pushBack'.format(self.__contextNamespace)](*args, **kwargs)



    def getDouble(self, *args, **kwargs):
        func = self.__dll_handle['{}getDouble'.format(self.__contextNamespace)]
        func.restype = c_double
        return func(*args, **kwargs)

    def getLength(self, *args, **kwargs):
        func = self.__dll_handle['{}getLength'.format(self.__contextNamespace)]
        func.restype = c_ulong
        return func(*args, **kwargs)

    def getKeys(self, *args, **kwargs):
        func = self.__dll_handle['{}getKeys'.format(self.__contextNamespace)]
        func.restype = POINTER(POINTER(c_char_p) * args[1])
        return func(*args, **kwargs)

    def getStr(self, *args, **kwargs):
        func = self.__dll_handle['{}getStr'.format(self.__contextNamespace)]
        func.restype = c_char_p
        return func(*args, **kwargs)

    def getStrSize(self, *args, **kwargs):
        func = self.__dll_handle['{}getStrSize'.format(self.__contextNamespace)]
        func.restype = c_ulong
        return func(*args, **kwargs)

    def getLong(self, *args, **kwargs):
        func = self.__dll_handle['{}getLong'.format(self.__contextNamespace)]
        func.restype = c_long
        return func(*args, **kwargs)

    def getUnsignedLong(self, *args, **kwargs):
        func = self.__dll_handle['{}getUnsignedLong'.format(self.__contextNamespace)]
        func.restype = c_ulong
        return func(*args, **kwargs)

    def getBool(self, *args, **kwargs):
        func = self.__dll_handle['{}getBool'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def getDataPtr(self, *args, **kwargs):
        func = self.__dll_handle['{}getDataPtr'.format(self.__contextNamespace)]
        func.restype = c_char_p
        return func(*args, **kwargs)


    def getByIndex(self, *args, **kwargs):
        func = self.__dll_handle['{}getByIndex'.format(self.__contextNamespace)]
        func.restype = c_void_p
        return func(*args, **kwargs)

    def getByKey(self, *args, **kwargs):
        func = self.__dll_handle['{}getByKey'.format(self.__contextNamespace)]
        func.restype = c_void_p
        return func(*args, **kwargs)

    def getOrInsertByKey(self, *args, **kwargs):
        func = self.__dll_handle['{}getOrInsertByKey'.format(self.__contextNamespace)]
        func.restype = c_void_p
        return func(*args, **kwargs)

    def isNone(self, *args, **kwargs):
        func = self.__dll_handle['{}isNone'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isArray(self, *args, **kwargs):
        func = self.__dll_handle['{}isArray'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isObject(self, *args, **kwargs):
        func = self.__dll_handle['{}isObject'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isBool(self, *args, **kwargs):
        func = self.__dll_handle['{}isBool'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isLong(self, *args, **kwargs):
        func = self.__dll_handle['{}isLong'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isUnsignedLong(self, *args, **kwargs):
        func = self.__dll_handle['{}isUnsignedLong'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isDouble(self, *args, **kwargs):
        func = self.__dll_handle['{}isDouble'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isString(self, *args, **kwargs):
        func = self.__dll_handle['{}isString'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def isDataPtr(self, *args, **kwargs):
        func = self.__dll_handle['{}isDataPtr'.format(self.__contextNamespace)]
        func.restype = c_bool
        return func(*args, **kwargs)

    def createProcessingBlock(self, *args, **kwargs):
        func = self.__dll_handle['TDVProcessingBlock_createProcessingBlock']
        func.restype = c_void_p
        return func(*args, **kwargs)

    def TDVProcessingBlock_destroyBlock(self, *args, **kwargs):
        self.__dll_handle['TDVProcessingBlock_destroyBlock'](*args, **kwargs)

    def TDVProcessingBlock_processContext(self, *args, **kwargs):
        self.__dll_handle['TDVProcessingBlock_processContext'](*args, **kwargs)
