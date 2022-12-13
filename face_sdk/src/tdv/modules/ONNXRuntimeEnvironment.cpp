#include <numeric>
#include <string>

#include <tdv/modules/ONNXRuntimeEnvironment.h>
#include <tdv/utils/rassert/RAssert.h>

#ifdef _WIN32
	#define WIDEN2(x) L ## x
	#define WIDEN(x) WIDEN2(x)
#else
	#define WIDEN(x) x
#endif

namespace tdv {

namespace modules {

using Context = tdv::data::Context;


void ONNXRuntimeEnvironment::OrtCheckStatus(OrtStatus* status) {
	if (status) {
		const char* msg = ort_api->GetErrorMessage(status);
		tdv::utils::rassert::tdv_error tdv_err(0xa9c6bf42, msg);
		ort_api->ReleaseStatus(status);
		throw tdv_err;
	}
}

ONNXRuntimeEnvironment::ONNXRuntimeEnvironment(const Context& config) : ort_api (OnnxRuntimeAdapter::GetInstance(config)->GetApi()), dynamic_batch(false)
{
	const bool enable_trace = config.get<bool>("enable_trace", false);
	const int intra_op_num_threads = config.get<long>("intra_op_num_threads", 1);
	const int execution_mode = config.get<long>("execution_mode", 0);
	const int inter_op_num_threads = config.get<long>("inter_op_num_threads", 1);
	const void* model_buffer = config["model_buffer"].get<char*>();
	const unsigned long model_buffer_size = config["model_buffer_size"].get<unsigned long>();

	// Initialize environment, could use ORT_LOGGING_LEVEL_VERBOSE to get more information
	// NOTE: Only one instance of env can exist at any point in time
	OrtCheckStatus(ort_api->CreateSessionOptions(&session_options));

	// Sets the number of threads used to parallelize the execution within nodes.
	OrtCheckStatus(ort_api->SetIntraOpNumThreads(session_options, intra_op_num_threads));
	if(execution_mode) {
		OrtCheckStatus(ort_api->SetSessionExecutionMode(session_options, static_cast<ExecutionMode>(execution_mode)));
		// Sets the number of threads used to parallelize the execution of the graph.
		OrtCheckStatus(ort_api->SetInterOpNumThreads(session_options, inter_op_num_threads));
	}


	// enable profiling, the argument is the prefix you want for the file
	if (enable_trace)
		OrtCheckStatus(ort_api->EnableProfiling(session_options, WIDEN("profile")));
	// Sets graph optimization level
	// Available levels are
	// ORT_DISABLE_ALL -> To disable all optimizations
	// ORT_ENABLE_BASIC -> To enable basic optimizations (Such as redundant node removals)
	// ORT_ENABLE_EXTENDED -> To enable extended optimizations (Includes level 1 + more complex optimizations like node fusions)
	// ORT_ENABLE_ALL -> To Enable All possible opitmizations
	OrtCheckStatus(ort_api->SetSessionGraphOptimizationLevel(session_options, ORT_ENABLE_ALL));

	if (config.get<bool>("enable_session_log", false))
	{
		OrtCheckStatus(ort_api->SetSessionLogId(session_options, "OnnxRuntime"));
		OrtCheckStatus(ort_api->SetSessionLogVerbosityLevel(session_options, ORT_LOGGING_LEVEL_VERBOSE));
		OrtCheckStatus(ort_api->SetSessionLogSeverityLevel(session_options, ORT_LOGGING_LEVEL_VERBOSE));
	}

	run_options = nullptr;
	if (config.get<bool>("enable_run_log", false))
	{
		OrtCheckStatus(ort_api->CreateRunOptions(&run_options));
		OrtCheckStatus(ort_api->RunOptionsSetRunTag(run_options, "OnnxRuntime"));
		OrtCheckStatus(ort_api->RunOptionsSetRunLogSeverityLevel(run_options, ORT_LOGGING_LEVEL_VERBOSE));
		OrtCheckStatus(ort_api->RunOptionsSetRunLogVerbosityLevel(run_options, ORT_LOGGING_LEVEL_VERBOSE));
	}

	if(config["use_cuda"].get<bool>()){
		int device_id = config.get<long>("device_id",  0);
#ifdef ONNXRT_OBSOLETE_API
		OrtCheckStatus(OnnxRuntimeAdapter::GetInstance(config)->SessionOptionsAppendExecutionProvider_CUDA(session_options, device_id));
#else
		OrtCUDAProviderOptions options;
		options.device_id = device_id;
		options.arena_extend_strategy = 0;
		options.cuda_mem_limit = 2 * 1024 * 1024 * 1024l;
		options.cudnn_conv_algo_search = OrtCudnnConvAlgoSearch::EXHAUSTIVE;
		options.do_copy_in_default_stream = 1;
		OrtCheckStatus(ort_api->SessionOptionsAppendExecutionProvider_CUDA(session_options, &options));
#endif
	}

#ifdef ANDROID
	if(config["use_nnapi"].get<bool>()){
#ifdef ONNXRT_OBSOLETE_API
		OrtCheckStatus(OnnxRuntimeAdapter::GetInstance(config)->SessionOptionsAppendExecutionProvider_Nnapi(session_options));
#else
		uint32_t nnapi_flags = 0;
		if (config.get<bool>("nnapi_disable_cpu", false))
			nnapi_flags |= 0x004; // NNAPI_FLAG_CPU_DISABLED
		if (config.get<bool>("nnapi_use_fp16", false))
			nnapi_flags |= 0x001; // NNAPI_FLAG_USE_FP16
		OrtCheckStatus(OnnxRuntimeAdapter::GetInstance(config)->SessionOptionsAppendExecutionProvider_Nnapi(session_options, nnapi_flags));
#endif
	}
#endif

	OrtCheckStatus(ort_api->GetAllocatorWithDefaultOptions(&allocator));
	OrtCheckStatus(ort_api->CreateSessionFromArray(OnnxRuntimeAdapter::GetInstance(config)->GetEnv(), model_buffer, model_buffer_size, session_options, &session));
	// TODO: extent on case of multiple inputs and outpus
	size_t numInputNodes, numOutputNodes;
	OrtCheckStatus(ort_api->SessionGetInputCount(session, &numInputNodes));
	OrtCheckStatus(ort_api->SessionGetOutputCount(session, &numOutputNodes));
	ONNXTensorElementDataType inputType, outputType;

	for(size_t i=0; i<numInputNodes; ++i)
	{
		char* input_name;
		OrtCheckStatus(ort_api->SessionGetInputName(session, i, allocator, &input_name));
		inputNames.push_back(input_name);

		OrtTypeInfo *typeinfo;
		const OrtTensorTypeAndShapeInfo *tensor_info;
		OrtCheckStatus(ort_api->SessionGetInputTypeInfo(session, i, &typeinfo));
		OrtCheckStatus(ort_api->CastTypeInfoToTensorInfo(typeinfo, &tensor_info));
		OrtCheckStatus(ort_api->GetTensorElementType(tensor_info, &inputType));
		inputTypes.push_back(inputType);

		size_t numInputDims;
		std::vector<int64_t> input_dims;
		OrtCheckStatus(ort_api->GetDimensionsCount(tensor_info, &numInputDims));
		input_dims.resize(numInputDims);
		OrtCheckStatus(ort_api->GetDimensions(tensor_info, input_dims.data(), numInputDims));

		dynamic_batch.push_back(input_dims[0] == -1);
		if (dynamic_batch.back())
			input_dims[0] = 1;	// default value

		// Quick fix for person dim of HAR input tensor
		// TODO: proper fix
		if (input_dims[1] == -1)
			input_dims[1] = 1;	// default value

		inputSizes.push_back(std::accumulate(std::begin(input_dims), std::end(input_dims), 1, std::multiplies<size_t>()));
		inputShapes.push_back(input_dims);
		ort_api->ReleaseTypeInfo(typeinfo);
	}
	for(size_t i=0; i<numOutputNodes; ++i)
	{
		char* output_name;
		OrtCheckStatus(ort_api->SessionGetOutputName(session, i, allocator, &output_name));
		outputNames.push_back(output_name);

		OrtTypeInfo *typeinfo;
		const OrtTensorTypeAndShapeInfo *tensor_info;
		OrtCheckStatus(ort_api->SessionGetOutputTypeInfo(session, i, &typeinfo));
		OrtCheckStatus(ort_api->CastTypeInfoToTensorInfo(typeinfo, &tensor_info));
		OrtCheckStatus(ort_api->GetTensorElementType(tensor_info, &outputType));
		outputTypes.push_back(outputType);
		ort_api->ReleaseTypeInfo(typeinfo);
	}
	OrtCheckStatus(ort_api->CreateCpuMemoryInfo(OrtArenaAllocator, OrtMemTypeDefault, &memory_info));
}

bool ONNXRuntimeEnvironment::adjust_batch_size(size_t input, long batch_size)
{
	if ((input < inputShapes.size()) && dynamic_batch[input])
	{
		if	(inputShapes[input][0] != batch_size)
		{
			inputShapes[input][0] = batch_size;
			inputSizes[input] = std::accumulate(std::begin(inputShapes[input]), std::end(inputShapes[input]), 1, std::multiplies<size_t>());
		}
		return true;
	}
	return false;
}

std::shared_ptr<uint8_t> ONNXRuntimeEnvironment::infer(std::vector<void*> input_data)
{
	std::vector<OrtValue*> input_tensors;
	std::vector<OrtValue*> output_tensors;
	for(size_t i=0; i < outputNames.size(); ++i)
		output_tensors.push_back(nullptr);

	size_t p_data_len = 0;
	for(size_t i = 0; i< inputNames.size(); ++i)
	{
		OrtValue *input_tensor = nullptr;
		unsigned long data_size = inputSizes[i]*OrtTypeTraits::tSize(inputTypes[i]);
		OrtCheckStatus(ort_api->CreateTensorWithDataAsOrtValue(memory_info,
															   static_cast<uint8_t*>(input_data[i])+p_data_len,
															   data_size,
															   inputShapes[i].data(), inputShapes[i].size(),
															   inputTypes[i], &input_tensor));
		input_tensors.push_back(input_tensor);
		p_data_len += data_size;
	}

	OrtCheckStatus(ort_api->Run(
			session,										// session
			run_options,									// run_options
			inputNames.data(),								// input_names
			(const OrtValue *const *)input_tensors.data(),	// input values
			input_tensors.size(),							// input_len
			outputNames.data(),								// output_names
			outputNames.size(),								// output_names_len
			output_tensors.data()));						// OrtValue** output

	for(auto input_tensor : input_tensors)
		ort_api->ReleaseValue(input_tensor);

	outputSizes.clear();
	outputShapes.clear();

	int isTensor;
	OrtTensorTypeAndShapeInfo* tensor_info;
	for(size_t i=0; i<outputNames.size(); ++i)
	{
		OrtCheckStatus(ort_api->IsTensor(output_tensors[i], &isTensor));
		if(isTensor)
		{
			OrtCheckStatus(ort_api->GetTensorTypeAndShape(output_tensors[i], &tensor_info));
			size_t numOutputDims;
			std::vector<int64_t> output_dims;
			OrtCheckStatus(ort_api->GetDimensionsCount(tensor_info, &numOutputDims));
			output_dims.resize(numOutputDims);
			OrtCheckStatus(ort_api->GetDimensions(tensor_info, output_dims.data(), numOutputDims));
			outputSizes.push_back(std::accumulate(output_dims.begin(), output_dims.end(), 1, std::multiplies<size_t>()));
			outputShapes.push_back(output_dims);
			ort_api->ReleaseTensorTypeAndShapeInfo(tensor_info);
		}
	}

	size_t buff_size{0};
	for(size_t i = 0; i < outputSizes.size(); ++i)
		buff_size += outputSizes[i]*OrtTypeTraits::tSize(outputTypes[i]);
	uint8_t* buff = static_cast<uint8_t*>(malloc(buff_size));
	if(!buff)
		throw std::bad_alloc();

	std::shared_ptr<uint8_t> output_buffer = std::shared_ptr<uint8_t>(buff, [](void *ptr){free(ptr);});
	p_data_len = 0;
	for(size_t i=0; i < outputNames.size(); ++i)
	{
		auto output_tensor = output_tensors[i];
		OrtCheckStatus(ort_api->IsTensor(output_tensor, &isTensor));
		if(isTensor)
		{
			void* parr = nullptr;
			OrtCheckStatus(ort_api->GetTensorMutableData(output_tensor, &parr));
			unsigned long data_size = outputSizes[i]*OrtTypeTraits::tSize(outputTypes[i]);
			memcpy((buff + p_data_len), parr, data_size);
			p_data_len += data_size;
			ort_api->ReleaseValue(output_tensor);
		}
	}
	return output_buffer;
}


const std::vector<std::vector<int64_t>>& ONNXRuntimeEnvironment::getInputShapes() const
{
	return inputShapes;
}

const std::vector<std::vector<int64_t>>& ONNXRuntimeEnvironment::getOutputShapes() const
{
	return outputShapes;
}

const std::vector<ONNXTensorElementDataType>& ONNXRuntimeEnvironment::getOutputTypes() const
{
	return outputTypes;
}

ONNXRuntimeEnvironment::~ONNXRuntimeEnvironment()
{
	for(auto inputName : inputNames)
		OrtCheckStatus(ort_api->AllocatorFree(allocator, inputName));
	for(auto outputName : outputNames)
		OrtCheckStatus(ort_api->AllocatorFree(allocator, outputName));
	ort_api->ReleaseMemoryInfo(memory_info);
	ort_api->ReleaseSession(session);
	ort_api->ReleaseSessionOptions(session_options);
}

}  // cnn namespace
}  // tdv namespace
