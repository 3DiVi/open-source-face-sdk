from ctypes import CDLL
from sys import platform

from .context import Context
from .processing_block import ProcessingBlock
from .dll_handle import DllHandle


class Service:

    def __init__(self,  dll_handle: CDLL, path_to_dir: str):
        self.path_to_dir = path_to_dir
        self.__dll_handle = dll_handle

    @staticmethod
    def create_service(path_to_dir: str):
        lib_path = f"/bin/open-source-sdk.dll" if platform == "win32" else f"/lib/libopen-source-sdk.so"

        dll_path = path_to_dir + lib_path

        dll_handle = DllHandle(CDLL(dll_path))

        return Service(dll_handle, path_to_dir)

    def create_processing_block(self, ctx: dict):
        ctx["@sdk_path"] = self.path_to_dir
        if "ONNXRuntime" not in ctx:
            ctx["ONNXRuntime"] = {"library_path": self.path_to_dir + ("/bin" if platform == "win32" else "/lib")}

        return ProcessingBlock(self.__dll_handle, ctx)

    def create_context(self, ctx) -> Context:
        ctr = Context(self.__dll_handle)
        ctr(ctx)
        return ctr