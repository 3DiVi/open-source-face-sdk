from ctypes import c_void_p
from typing import Union

from .complex_object import ComplexObject
from .exception_check import check_exception, make_exception
from .dll_handle import DllHandle
from .context import Context
from .error import Error


class ProcessingBlock(ComplexObject):
    def __init__(self, handle: DllHandle, ctx):
        exception = make_exception()
        meta_ctx = Context(handle)

        meta_ctx(ctx)

        impl = handle.createProcessingBlock(meta_ctx._impl, exception)

        check_exception(exception, handle)

        super().__init__(handle, c_void_p(impl))

    def __del__(self):
        exception = make_exception()

        self._dll_handle.TDVProcessingBlock_destroyBlock(self._impl, exception)

        check_exception(exception, self._dll_handle)

    def __call__(self, ctx: Union[dict, Context]):
        if isinstance(ctx, dict):
            self.__call_dicts(ctx)
        elif isinstance(ctx, Context):
            self.__call_ctx(ctx)
        else:
            raise Error(0xa341de35, "Wrong type of ctx")

    def __call_dicts(self, ctx: dict):
        exception = make_exception()
        meta_ctx = Context(self._dll_handle)
        meta_ctx(ctx)

        self._dll_handle.TDVProcessingBlock_processContext(self._impl, meta_ctx._impl, exception)

        check_exception(exception, self._dll_handle)

        new_keys_dict = set(meta_ctx.keys()) - set(ctx.keys())
        for key in new_keys_dict:
            ctx[key] = self.get_output_data(meta_ctx[key])

    def __call_ctx(self, ctx: Context):
        exception = make_exception()

        self._dll_handle.TDVProcessingBlock_processContext(self._impl, ctx._impl, exception)

        check_exception(exception, self._dll_handle)

    def get_output_data(self, meta_ctx: Context):
        if meta_ctx.is_array():
            return [self.get_output_data(meta_ctx[i]) for i in range(len(meta_ctx))]

        if meta_ctx.is_object():
            return {key: self.get_output_data(meta_ctx[key]) for key in meta_ctx.keys()}

        return meta_ctx.get_value()
