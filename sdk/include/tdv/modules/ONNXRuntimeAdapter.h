#ifndef ONNXRUNTIMEADAPTER_H
#define ONNXRUNTIMEADAPTER_H

#include <mutex>
#include <memory>

#ifdef _WIN32
#define _In_
#define _In_z_
#define _In_opt_
#define _In_opt_z_
#define _Out_
#define _Outptr_
#define _Out_opt_
#define _Inout_
#define _Inout_opt_
#define _Frees_ptr_opt_
#define _Ret_maybenull_
#define _Ret_notnull_
#define _Check_return_
#define _Outptr_result_maybenull_
#define _In_reads_(X)
#define _Inout_updates_all_(X)
#define _Out_writes_bytes_all_(X)
#define _Out_writes_all_(X)
#define _Success_(X)
#define _Outptr_result_buffer_maybenull_(X)
#define _Return_type_success_(X)
#endif

#include <onnxruntime_c_api.h>

#include <tdv/data/Context.h>

#ifdef _WIN32
	#include <windows.h>
	#define HANDLE HMODULE
#else
	#include <dlfcn.h>
	#define HANDLE void*
#endif

namespace tdv {

namespace modules {

class OnnxRuntimeAdapter{

	HANDLE handle;
	const OrtApi *ort_api = nullptr;
	OrtEnv *env = nullptr;
	static std::unique_ptr<OnnxRuntimeAdapter> pinstance_;
	static std::unique_ptr<OnnxRuntimeAdapter> pinstance_cuda_;
	static std::mutex mutex_;
	static std::mutex mutex_cuda_;

protected:
	OnnxRuntimeAdapter(const std::string& onnx_path, bool use_cuda);

public:
	OnnxRuntimeAdapter(const OnnxRuntimeAdapter&) = delete;
	OnnxRuntimeAdapter& operator=(const OnnxRuntimeAdapter&) = delete;
	~OnnxRuntimeAdapter();

	static OnnxRuntimeAdapter *GetInstance(const tdv::data::Context& ctx);
	const OrtApi* GetApi();
	const OrtEnv* GetEnv();
	OrtStatus* SessionOptionsAppendExecutionProvider_CUDA(OrtSessionOptions* options, int device_id);
	OrtStatus* SessionOptionsAppendExecutionProvider_Nnapi(OrtSessionOptions* options, uint32_t nnapi_flags=0);
};

}
}


#endif // ONNXRUNTIMEADAPTER_H
