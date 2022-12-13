#include <sstream>

#include <tdv/modules/ONNXRuntimeAdapter.h>
#include <tdv/utils/rassert/RAssert.h>

#define STRINGIZE2(s) #s
#define STRINGIZE(s) STRINGIZE2(s)
#define ONNX_HASH_STRINGS STRINGIZE(ONNX_HASHS)

#ifdef ANDROID
#include <android/log.h>
#endif

#ifdef _WIN32
	#include <windows.h>

namespace {

	std::string GetLastErrorAsString()
	{
		//Get the error message ID, if any.
		DWORD errorMessageID = ::GetLastError();
		if(errorMessageID == 0) {
			return std::string(); //No error message has been recorded
		}

		LPSTR messageBuffer = nullptr;

		//Ask Win32 to give us the string version of that message ID.
		//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		//Copy the error message into a std::string.
		std::string message(messageBuffer, size);

		//Free the Win32's string's buffer.
		LocalFree(messageBuffer);

		return message;
	}
}

	#define ONNX_RESOLVE(x) GetProcAddress(handle, x)
	#define LOAD_LIBRARY(x) LoadLibrary(x)
	#define FREE_LIBRARY(x) FreeLibrary(x)
	#define GET_ERROR_MSG GetLastErrorAsString
	#define HANDLE HMODULE
	#define ONNX_NAME "onnxruntime.dll"
	#define ONNX_CUDA_NAME "onnxruntime_cuda.dll"
	#define SLASH "\\"


#else
	#include <dlfcn.h>

	#define ONNX_RESOLVE(x) dlsym(handle, x)

#if defined(__ANDROID__) || defined(__ios__)
	#define LOAD_LIBRARY(x) dlopen(x, RTLD_NOW|RTLD_LOCAL)
#else
	#define LOAD_LIBRARY(x) dlopen(x, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND)
#endif
	#define FREE_LIBRARY(x) dlclose(x)
	#define GET_ERROR_MSG dlerror
	#define HANDLE void*

#ifdef __ios__
	#define ONNX_NAME "onnxruntime.framework/libonnxruntime.dylib"
	#define ONNX_CUDA_NAME "onnxruntime.framework/libonnxruntime.dylib"
#else
	#define ONNX_NAME "libonnxruntime.so"
	#define ONNX_CUDA_NAME "libonnxruntime_cuda.so"
#endif
	#define SLASH "/"
#endif

namespace tdv {

namespace modules {

OnnxRuntimeAdapter* OnnxRuntimeAdapter::pinstance_{nullptr};
OnnxRuntimeAdapter* OnnxRuntimeAdapter::pinstance_cuda_{nullptr};

std::mutex OnnxRuntimeAdapter::mutex_;
std::mutex OnnxRuntimeAdapter::mutex_cuda_;


OnnxRuntimeAdapter::OnnxRuntimeAdapter(const std::string& onnx_path, bool use_cuda) {
	handle = LOAD_LIBRARY(onnx_path.c_str());
	RHAssert2(0x032ad038, handle, "ERROR: " + onnx_path + " could not be loaded - " + GET_ERROR_MSG());
	OrtApiBase *(*ort_api_base)() = (OrtApiBase* (*)())ONNX_RESOLVE("OrtGetApiBase");
	RHAssert2(0x4a7f85d1, ort_api_base, GET_ERROR_MSG());
	ort_api = ort_api_base()->GetApi(ORT_API_VERSION);
	// Initialize environment, could use ORT_LOGGING_LEVEL_VERBOSE to get more information
	// NOTE: Only one instance of env can exist at any point in time
	auto status = ort_api->CreateEnv(ORT_LOGGING_LEVEL_ERROR, "OnnxRuntime", &env);
	if (status) {
		const char* msg = ort_api->GetErrorMessage(status);
		tdv::utils::rassert::tdv_error tdv_err(0x312bca42, msg);
		ort_api->ReleaseStatus(status);
		throw tdv_err;
	}
}

OnnxRuntimeAdapter::~OnnxRuntimeAdapter() {
		ort_api->ReleaseEnv(env);
		FREE_LIBRARY(handle);
}

const OrtApi* OnnxRuntimeAdapter::GetApi() {
	RHAssert2(0xd6d1b198, ort_api, "ERROR: uninitialized Runtime");
	return ort_api;
}

// obsolete, special for onnxruntime 1.4
OrtStatus* OnnxRuntimeAdapter::SessionOptionsAppendExecutionProvider_CUDA(OrtSessionOptions* options, int device_id) {
	RHAssert2(0xd68fb198, options, "ERROR: null OrtSessionOptions in SessionOptionsAppendExecutionProvider_CUDA");
	RHAssert2(0xdaaab198, handle, "ERROR: null handle in SessionOptionsAppendExecutionProvider_CUDA");
	OrtStatus*(*ort_append_cuda_ep)(OrtSessionOptions *, int) = (OrtStatus*(*)(OrtSessionOptions *, int))ONNX_RESOLVE("OrtSessionOptionsAppendExecutionProvider_CUDA");
	RHAssert2(0x9384aad1, ort_append_cuda_ep, GET_ERROR_MSG());
	return ort_append_cuda_ep(options, device_id);
}

OrtStatus* OnnxRuntimeAdapter::SessionOptionsAppendExecutionProvider_Nnapi(OrtSessionOptions* options, uint32_t nnapi_flags) {
	RHAssert2(0xd1c402dc, options, "ERROR: null OrtSessionOptions in SessionOptionsAppendExecutionProvider_Nnapi");
	RHAssert2(0xd1c402dc, handle, "ERROR: null handle in SessionOptionsAppendExecutionProvider_Nnapi");
#ifdef ONNXRT_OBSOLETE_API
		OrtStatus*(*ort_append_nnapi_ep)(OrtSessionOptions *) = (OrtStatus*(*)(OrtSessionOptions *))ONNX_RESOLVE("OrtSessionOptionsAppendExecutionProvider_Nnapi");
		RHAssert2(0x4aeea0fc, ort_append_nnapi_ep, GET_ERROR_MSG());
		return ort_append_nnapi_ep(options);
#else
		OrtStatus*(*ort_append_nnapi_ep)(OrtSessionOptions *, uint32_t) = (OrtStatus*(*)(OrtSessionOptions *, uint32_t))ONNX_RESOLVE("OrtSessionOptionsAppendExecutionProvider_Nnapi");
		RHAssert2(0x278227eb, ort_append_nnapi_ep, GET_ERROR_MSG());
		return ort_append_nnapi_ep(options, nnapi_flags);
#endif
}

OnnxRuntimeAdapter *OnnxRuntimeAdapter::GetInstance(const tdv::data::Context& config)
{
	bool use_cuda = config.get<bool>("use_cuda", false);
	if (!use_cuda)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (pinstance_ == nullptr)
		{
			auto library_path = config.find("library_path");
			pinstance_ = new OnnxRuntimeAdapter(library_path != config.end() ?
												(*library_path).get<std::string>() + SLASH + ONNX_NAME : ONNX_NAME,
												use_cuda);
		}
		return pinstance_;
	}
	else
	{
		std::lock_guard<std::mutex> lock(mutex_cuda_);
		if(pinstance_cuda_ == nullptr)
		{
			auto library_path = config.find("library_path");
			pinstance_cuda_ = new OnnxRuntimeAdapter(library_path != config.end() ?
												(*library_path).get<std::string>() + SLASH + ONNX_CUDA_NAME :
												ONNX_CUDA_NAME, use_cuda);
		}
		return pinstance_cuda_;
	}
}

const OrtEnv* OnnxRuntimeAdapter::GetEnv(){
	RHAssert2(0xb3d16198, env, "ERROR: uninitialized Ort Environment");
	return env;
}


}
}
