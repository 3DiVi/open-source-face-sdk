#ifndef ONNXRuntimeEnvironment_H
#define ONNXRuntimeEnvironment_H

#include <tdv/modules/ONNXRuntimeAdapter.h>

#include <tdv/data/Context.h>


namespace tdv {
namespace modules {

struct OrtTypeTraits
{
	static size_t tSize(ONNXTensorElementDataType oType){
		switch (oType)
		{
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
			return sizeof(float);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE:
			return sizeof(double);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL:
			return sizeof(bool);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
			return sizeof(int8_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
			return sizeof(uint8_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
			return sizeof(int16_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
			return sizeof(uint16_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
			return sizeof(int32_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
			return sizeof(uint32_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
			return sizeof(int64_t);
		case ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64:
			return sizeof(uint64_t);
		default:
			return 0;
		}
	}
};

class ONNXRuntimeEnvironment
{
public:
	ONNXRuntimeEnvironment(const tdv::data::Context& config);

	ONNXRuntimeEnvironment(const ONNXRuntimeEnvironment&) = delete;
	ONNXRuntimeEnvironment& operator=(const ONNXRuntimeEnvironment&) = delete;

	~ONNXRuntimeEnvironment();

	std::shared_ptr<uint8_t> infer(std::vector<void*> input_data);
	bool adjust_batch_size(size_t input, int64_t batch_size);

	const std::vector<std::vector<int64_t>>& getInputShapes() const;
	const std::vector<std::vector<int64_t>>& getOutputShapes() const;
	const std::vector<ONNXTensorElementDataType>& getOutputTypes() const;
	const std::vector<bool>& getDynamicBatchEnabled() const;


private:
	void OrtCheckStatus(OrtStatus* status);

	const OrtApi* ort_api;
	OrtSessionOptions* session_options;
	OrtSession* session;
	OrtAllocator* allocator;
	OrtMemoryInfo* memory_info;
	OrtRunOptions* run_options;

	std::vector<char*> inputNames;
	std::vector<ONNXTensorElementDataType> inputTypes;
	std::vector<std::vector<int64_t>> inputShapes;
	std::vector<size_t> inputSizes;

	std::vector<char*> outputNames;
	std::vector<ONNXTensorElementDataType> outputTypes;
	std::vector<std::vector<int64_t>> outputShapes;
	std::vector<size_t> outputSizes;

	std::vector<bool> dynamic_batch;
};

}  // modules namespace
}  // tdv namespace

#endif // ONNXRuntimeEnvironment_H
