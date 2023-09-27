import os
from ctypes import CDLL
from sys import platform
from pathlib import Path

from .models import make_model_paths, download_models
from .context import Context
from .processing_block import ProcessingBlock
from .dll_handle import DllHandle


class Service:
    """
    Provides ProcessingBlock and Context creation
    """
    def __init__(self, dll_handle: CDLL, path_to_dir: str, binaries_path: str):
        self.__dll_handle = dll_handle
        self.path_to_dir = path_to_dir
        self.binaries_path = binaries_path

    @staticmethod
    def create_service(path_to_dir: str = ""):
        """
        Create a Service object
        :param path_to_dir: Path to directory with data/models
        :return: Service
        """
        package_path = Path(__file__).parent.parent

        binaries_path = os.path.join(
            package_path,
            "for_windows" if platform == "win32" else "for_linux"
        )
        sdk_library_path = os.path.join(
            binaries_path,
            "open_source_sdk",
            "open_source_sdk.dll" if platform == "win32" else "libopen_source_sdk.so"
        )

        dll_handle = DllHandle(CDLL(sdk_library_path))

        if len(path_to_dir) == 0:
            path_to_dir = str(package_path)

        return Service(dll_handle, path_to_dir, binaries_path)

    def create_processing_block(self, ctx: dict) -> ProcessingBlock:
        """
        Create a ProcessingBlock object
        :param ctx: Config context with unit_type
        :return: ProcessingBlock
        """
        unit_type = str(ctx["unit_type"])

        ctx["@sdk_path"] = self.path_to_dir

        if "ONNXRuntime" not in ctx:
            ctx["ONNXRuntime"] = {
                "library_path": os.path.join(
                    self.binaries_path,
                    "onnxruntime-windows-x86-64-shared-install-dir" if platform == "win32" else "onnxruntime-linux-x86-64-shared-install-dir"
                )
            }

        if len(unit_type) != 0:
            for model_path in make_model_paths(self.path_to_dir, unit_type):
                if not os.path.exists(model_path):
                    download_models(self.path_to_dir, unit_type)

                    break

        return ProcessingBlock(self.__dll_handle, ctx)

    def create_context(self, ctx) -> Context:
        """
        Create a Context object
        :param ctx: dict
        :return: Context
        """
        ctr = Context(self.__dll_handle)
        ctr(ctx)
        return ctr
