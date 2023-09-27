#ifndef ONNXMODULE_H
#define ONNXMODULE_H

#include <sys/stat.h>
#include <iostream>
#include <fstream>

#include <tdv/modules/ONNXRuntimeEnvironment.h>
#include <tdv/modules/ProcessingBlock.h>


namespace tdv {

namespace modules {

using Context = tdv::data::Context;


class InputAdapter
{
public:
	virtual void convertInput(data::Context& inputData, data::Context& workData) = 0;
	virtual void convertOutput(data::Context& workData, data::Context& outputData) = 0;
};

class DefaultInputAdapter: public InputAdapter
{
public:
	virtual void convertInput(data::Context& inputData, data::Context& workData) override
	{
		const bool isMultipleInput = inputData.isArray();
		if (isMultipleInput)
			throw std::runtime_error("multiple input is not supported");
		workData = std::move(inputData);
	}
	virtual void convertOutput(data::Context& workData, data::Context& outputData) override
	{
		outputData = std::move(workData);
	}
};

class MultiInputAdapter: public InputAdapter
{
public:
	virtual void convertInput(data::Context& inputData, data::Context& workData) override
	{
		const bool isMultipleInput = inputData.isArray();

		if (isMultipleInput)
		{
			workData["input"] = std::move(inputData);
			workData["input_type"] = "multiple";
		} else
		{
			workData["input"].push_back(std::move(inputData));
			workData["input_type"] = "single";
		}
	}

	virtual void convertOutput(data::Context& workData, data::Context& outputData) override
	{
		auto inputType = workData.get<std::string>("input_type", "single");
		const bool isMultipleInput = (inputType == "multiple");

		auto &inputData = workData["input"];
		if (isMultipleInput) 
		{
			outputData = std::move(inputData);
			//workData.erase("input");
		} else
		{
			outputData = std::move(inputData[0]);
			//inputData.erase(inputData[0]);
		}
	}
};


template<typename Derived>
class ONNXModule : public ProcessingBlock {

public:
	ONNXModule(const tdv::data::Context& config, 
		std::shared_ptr<InputAdapter> inputAdapter = std::make_shared<DefaultInputAdapter>());

	virtual void operator ()(tdv::data::Context& data) override;

protected:
	const std::vector<std::vector<int64_t>>& getInputShapes() const {
		return ort_env->getInputShapes();
	}

	const std::vector<std::vector<int64_t>>& getOutputShapes() const {
		return ort_env->getOutputShapes();
	}

	std::vector<int> getOutputTypes() const {
		std::vector<int> outTypes;
		auto types = ort_env->getOutputTypes();
		std::copy(types.begin(), types.end(), std::back_inserter(outTypes));
		return outTypes;
	}
private:
	void readToBuffer(const std::string& filePath, char *result, int buffer_size);

	Derived* self()
	{
		return static_cast<Derived*>(this);
	}

	virtual void preprocess(tdv::data::Context& data) {
		return;
	}

	virtual void postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
		return;
	}

	std::unique_ptr<ONNXRuntimeEnvironment> ort_env;
	std::shared_ptr<char> model_buffer;
	std::shared_ptr<InputAdapter> _inputAdapter;
};

template<typename Derived>
ONNXModule<Derived>::ONNXModule(const tdv::data::Context& config, std::shared_ptr<InputAdapter> inputAdapter)
	: _inputAdapter(inputAdapter)
{
	const std::string filePath = config.at("model_path").get<std::string>();
	struct stat sb{};
	if (!stat(filePath.c_str(), &sb)) {
		model_buffer = std::shared_ptr<char>(static_cast<char*>(malloc(sb.st_size)), [](void* ptr){free(ptr);});
	} else {
		throw std::runtime_error("model file not found");
	}

	readToBuffer(filePath, model_buffer.get(), sb.st_size);

	Context modelConfig = config["ONNXRuntime"];
	modelConfig["model_buffer"] = model_buffer.get();
	modelConfig["model_buffer_size"] = static_cast<uint64_t>(sb.st_size);
#if defined( ANDROID ) || defined( __ios__ )
	if (config.get<bool>("use_cuda", false))
		std::cerr << "Warning: Mobile devices do not support CUDA\n";
	modelConfig["use_cuda"] = false;
#else
	modelConfig["use_cuda"] = config.get<bool>("use_cuda", false);
#endif

#if defined( ANDROID )
	modelConfig["use_nnapi"] = config.get<bool>("use_nnapi", false);
#endif
	modelConfig["batch_size"] = config.get<size_t>("batch_size", 1UL);

	ort_env = std::unique_ptr<ONNXRuntimeEnvironment>(new ONNXRuntimeEnvironment(modelConfig));
}

template<typename Derived>
void ONNXModule<Derived>::operator ()(tdv::data::Context& data) {
	tdv::data::Context workData;
	_inputAdapter->convertInput(data, workData);

	size_t inputShapesSize = getInputShapes().size();
	const auto &dynamicBatchEnabled = ort_env->getDynamicBatchEnabled();
	auto inputTypeIsMultiple = 
		(workData.get<std::string>("input_type", "single") == "multiple");

	do
	{
		preprocess(workData);

		if (workData.find("objects@input") == workData.end())
			break;

		Context& input_array = workData.at("objects@input");

		std::vector<void*> input_data;
		auto inputsCount = std::min<size_t>(inputShapesSize, input_array.size());
		for (size_t i = 0; i < inputsCount; ++i)
		{
			auto batchSize = input_array[i].get<size_t>("batch_size", 1);

			if (dynamicBatchEnabled[i])
				ort_env->adjust_batch_size(i, batchSize);
			else
				if (batchSize > 1)
					throw std::runtime_error("model input type is static but requested dynamic batch");

			input_data.push_back(input_array[i].at("input_ptr").get<std::shared_ptr<unsigned char>>().get());
		}

		std::shared_ptr<uint8_t> out_ptr = ort_env->infer(input_data);

		postprocess(out_ptr, workData);
		workData.erase("objects@input");
	} while (inputTypeIsMultiple);	

	_inputAdapter->convertOutput(workData, data);
}


template<typename Derived>
void ONNXModule<Derived>::readToBuffer(const std::string& filePath, char *result, int buffer_size)
{
	std::ifstream file_stream;
	file_stream.open(filePath.c_str(), std::ifstream::binary);
	file_stream.read(result, buffer_size);
	file_stream.close();
}

}
}
#endif // ONNXMODULE_H
