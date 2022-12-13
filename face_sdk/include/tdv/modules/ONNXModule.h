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

template<typename Derived>
class ONNXModule : public ProcessingBlock {

public:
	ONNXModule(const tdv::data::Context& config);

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
};

template<typename Derived>
ONNXModule<Derived>::ONNXModule(const tdv::data::Context& config)
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
	modelConfig["model_buffer_size"] = static_cast<unsigned long>(sb.st_size);
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
	ort_env = std::unique_ptr<ONNXRuntimeEnvironment>(new ONNXRuntimeEnvironment(modelConfig));
}


template<typename Derived>
void ONNXModule<Derived>::operator ()(tdv::data::Context& data) {
	preprocess(data);
	Context& input_array = data.at("objects@input");
	std::vector<void*> input_data;
	for (size_t i = 0; i < input_array.size(); ++i)
	{
		auto bs_iter = input_array[i].find("batch_size");
		if (bs_iter != input_array[i].end())
			ort_env->adjust_batch_size(i, (*bs_iter).get<size_t>());
		input_data.push_back(input_array[i].at("input_ptr").get<std::shared_ptr<unsigned char>>().get());
	}
	std::shared_ptr<uint8_t> out_ptr = ort_env->infer(input_data);
	postprocess(out_ptr, data);
	data.erase("objects@input");
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
