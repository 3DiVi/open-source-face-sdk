from multipledispatch import dispatch
from ctypes import c_char_p, c_void_p
from ctypes import c_int32, c_bool, c_long, c_ulong, c_double, POINTER

from .exception_check import check_exception, make_exception
from .complex_object import ComplexObject
from .dll_handle import DllHandle


class Context(ComplexObject):

    def __init__(self, handle: DllHandle, the_impl=None):
        self.__weak_ = False
        self.data = None
        if the_impl is None:
            exception = make_exception()

            the_impl = c_void_p(handle.create(exception))

            check_exception(exception, handle)

        super(Context, self).__init__(handle, the_impl)

    def __del__(self):
        if not self.__weak_:
            exception = make_exception()
            self._dll_handle.destroy(self._impl, exception)

            check_exception(exception, self._dll_handle)

    def __call__(self, ctx):
        self.parser(ctx)

    @dispatch(int)
    def __getitem__(self, key):
        """
        Get by index
        :param key: Context array index
        :return: Context
        """
        return self.__getByIndex(key)

    @dispatch(str)
    def __getitem__(self, key):
        """
        Get by key
        :param key: Context key
        :return: Context
        """
        return self.__getByKey(key)

    def __setitem__(self, key, value):
        """
        Setter
        :param key: Context key
        :param value: Context value
        :return: Context
        """
        return self.__getOrInsertByKey(key).parser(value)

    def __len__(self):
        """
        Get array Context size
        :return: Context array size
        """
        return self.__getLength()

    def __iter__(self):
        self.n = 0
        return self

    def __next__(self):
        if self.n >= self.__getLength():
            raise StopIteration

        result = self.__getByIndex(self.n)
        self.n += 1
        return result

    def set_weak(self, weak):
        self.__weak_ = weak
        return self

    def push_back(self, data):
        """
        Add Context to array
        :param data: New Context
        """
        self.__pushBack(data)

    def __setStr(self, value: str):
        """
        Set Context value
        :param value: String Context value
        """
        exception = make_exception()

        self._dll_handle.putStr(self._impl, c_char_p(bytes(value, "ascii")), exception)

        check_exception(exception, self._dll_handle)

    def __getStr(self):
        """
        Get Context value
        :return: String value
        """
        exception = make_exception()

        buff = c_char_p()
        str1 = self._dll_handle.getStr(self._impl, buff, exception)

        check_exception(exception, self._dll_handle)
        return str(str1, "ascii")

    def __setDouble(self, value: float):
        """
        Set Context value
        :param value: Double Context value
        """
        exception = make_exception()

        self._dll_handle.putDouble(self._impl, c_double(value), exception)

        check_exception(exception, self._dll_handle)

    def __getDouble(self):
        """
        Get Context value
        :return: Double value
        """
        exception = make_exception()

        value = self._dll_handle.getDouble(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def __setBool(self, value: bool):
        """
        Set Context value
        :param value: Bool Context value
        """
        exception = make_exception()

        self._dll_handle.putBool(self._impl, c_bool(value), exception)

        check_exception(exception, self._dll_handle)

    def __setDataPtr(self, value: bytes):
        """
        Set Context value
        :param value: Array of bytes
        """
        exception = make_exception()

        self._dll_handle.putDataPtr(self._impl, c_char_p(value), c_ulong(len(value)), exception)
        check_exception(exception, self._dll_handle)

    def getDataPtr(self):
        """
        Get Context value
        :return: Array of bytes
        """
        exception = make_exception()

        result = self._dll_handle.getDataPtr(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return result

    def __getBool(self):
        """
        Get Context value
        :return: Bool value
        """
        exception = make_exception()

        value = self._dll_handle.getBool(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def __setLong(self, value: int):
        """
        Set Context value
        :param value: Long Context value
        """
        exception = make_exception()

        self._dll_handle.putLong(self._impl, c_long(value), exception)

        check_exception(exception, self._dll_handle)

    def __getLong(self):
        """
        Get Context value
        :return: Long value
        """
        exception = make_exception()

        value = self._dll_handle.getLong(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def __getUnsignedLong(self):
        """
        Get Context value
        :return: Unsigned long value
        """
        exception = make_exception()

        value = self._dll_handle.getUnsignedLong(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def __getLength(self):
        """
        Get array Context size
        :return: Context array size
        """
        exception = make_exception()

        value = self._dll_handle.getLength(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def getStrSize(self):
        """
        Get string size
        :return: Context string value size
        """
        exception = make_exception()

        value = self._dll_handle.getStrSize(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def __getOrInsertByKey(self, key: str):
        """
        Returns or insert and returns Context
        :param key: Map key
        :return: Context
        """
        exception = make_exception()

        new_impl = self._dll_handle.getOrInsertByKey(self._impl, c_char_p(bytes(key, "ascii")), exception)

        check_exception(exception, self._dll_handle)

        return Context(self._dll_handle, c_void_p(new_impl)).set_weak(True)

    def __getByKey(self, key: str):
        """
        Get Context from map by key
        :param key: Map key
        :return: Context
        """
        exception = make_exception()

        new_impl = self._dll_handle.getByKey(self._impl, c_char_p(bytes(key, "ascii")), exception)

        check_exception(exception, self._dll_handle)

        return Context(self._dll_handle, c_void_p(new_impl)).set_weak(True)

    def __getByIndex(self, key: int):
        """
        Get Context from array by index
        :param key: Array index
        :return: Context
        """
        exception = make_exception()

        new_impl = self._dll_handle.getByIndex(self._impl, c_int32(key), exception)

        check_exception(exception, self._dll_handle)

        return Context(self._dll_handle, c_void_p(new_impl)).set_weak(True)

    def clone(self):
        """
        Copy Context
        :return: New Context
        """
        exception = make_exception()

        impl = self._dll_handle.clone(self._impl, exception)

        check_exception(exception, self._dll_handle)

        return Context(self._dll_handle, c_void_p(impl))

    def __pushBack(self, data):
        """
        Add Context to array
        :param data: New Context
        """
        exception = make_exception()

        self._dll_handle.pushBack(self._impl, data._impl, c_bool(True), exception)

        check_exception(exception, self._dll_handle)

    @dispatch(dict)
    def parser(self, ctx: dict):
        """
        Add all values of dict to Context map
        :param ctx: Context key - value
        """
        for key in ctx.keys():
            self.__getOrInsertByKey(key)(ctx[key])

    @dispatch(list)
    def parser(self, ctx: list):
        """
        Add all values of list to Context map
        :param ctx: Context array values
        """
        for value in ctx:
            ctt = Context(self._dll_handle)
            ctt.parser(value)
            self.__pushBack(ctt)

    @dispatch(str)
    def parser(self, ctx: str):
        """
        Set Context value
        :param ctx: String Context value
        """
        self.__setStr(ctx)

    @dispatch(int)
    def parser(self, ctx: int):
        """
        Set Context value
        :param ctx: Int Context value
        """
        self.__setLong(ctx)

    @dispatch(float)
    def parser(self, ctx: float):
        """
        Set Context value
        :param ctx: Float Context value
        """
        self.__setDouble(ctx)

    @dispatch(bool)
    def parser(self, ctx: bool):
        """
        Set Context value
        :param ctx: Bool Context value
        """
        self.__setBool(ctx)

    @dispatch(bytes)
    def parser(self, ctx: bytes):
        """
        Set Context value
        :param ctx: Array of bytes
        """
        self.__setDataPtr(ctx)

    def __get_keys(self) -> list:
        """
        Get all Context map keys
        :return: Context keys
        """
        exception = make_exception()

        cout_keys = self.__getLength()
        buf = POINTER(c_char_p)
        
        p_value_array = self._dll_handle.getKeys(self._impl, cout_keys, c_ulong(cout_keys), exception)
        check_exception(exception, self._dll_handle)

        result = [buf(i)[0].decode() for i in p_value_array[0]]

        for i in p_value_array[0]:
            self._dll_handle.freePtr(i)
        self._dll_handle.freePtr(p_value_array[0])

        return result

    def keys(self) -> list:
        """
        Get all Context map keys
        :return: Context keys
        """
        return self.__get_keys()

    # type checkers

    def is_none(self) -> bool:
        """
        Check Context value is None
        :return: Is Context None
        """
        exception = make_exception()

        value = self._dll_handle.isNone(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_array(self) -> bool:
        """
        Check Context value is array
        :return: Is Context array
        """
        exception = make_exception()

        value = self._dll_handle.isArray(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_object(self) -> bool:
        """
        Check Context value is object
        :return: Is Context object
        """
        exception = make_exception()

        value = self._dll_handle.isObject(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_bool(self) -> bool:
        """
        Check Context value is bool
        :return: Is Context bool
        """
        exception = make_exception()

        value = self._dll_handle.isBool(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_long(self) -> bool:
        """
        Check Context value is long
        :return: Is Context long
        """
        exception = make_exception()

        value = self._dll_handle.isLong(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_unsigned_long(self) -> bool:
        """
        Check Context value is unsigned long
        :return: Is Context unsigned long
        """
        exception = make_exception()

        value = self._dll_handle.isUnsignedLong(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_double(self) -> bool:
        """
        Check Context value is double
        :return: Is Context double
        """
        exception = make_exception()

        value = self._dll_handle.isDouble(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_string(self) -> bool:
        """
        Check Context value is string
        :return: Is Context string
        """
        exception = make_exception()

        value = self._dll_handle.isString(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def is_data_ptr(self) -> bool:
        """
        Check Context value is array of bytes
        :return: Is Context array of bytes
        """
        exception = make_exception()

        value = self._dll_handle.isDataPtr(self._impl, exception)

        check_exception(exception, self._dll_handle)
        return value

    def get_value(self):
        """
        Get current Context value
        :return: Current Context value
        """
        if self.is_none():
            return None
        if self.is_bool():
            return self.__getBool()
        if self.is_string():
            return self.__getStr()
        if self.is_long():
            return self.__getLong()
        if self.is_unsigned_long():
            return self.__getUnsignedLong()
        if self.is_double():
            return self.__getDouble()
        if self.is_data_ptr():
            return self.getDataPtr()

        return None
