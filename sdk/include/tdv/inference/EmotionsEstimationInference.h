#ifndef EMOTIONS_ESTIMATION_INFERENCE_H
#define EMOTIONS_ESTIMATION_INFERENCE_H

#include <tdv/inference/BaseEstimationInference.h>

namespace {

std::vector<double> softmaxFunction(const std::vector<float>& predict) {
	double sum{0};
	std::vector<double> softmax_predict(predict.size());
	for(int i = 0; i < predict.size(); i++)
		softmax_predict[i] = exp(double(predict[i]));
	
	for(int i = 0; i < softmax_predict.size(); i++)
		sum += softmax_predict[i];

	for(int i = 0; i < softmax_predict.size(); i++)
		softmax_predict[i] /= sum;

	return softmax_predict;
}

}


namespace tdv {

namespace modules {

template <typename Impl, TypeCrop typeCrop = SIMPLE_CROP>
class EmotionsEstimationInference : public BaseEstimationInference<Impl, typeCrop>
{
public:
	EmotionsEstimationInference(const tdv::data::Context& config);
private:
	friend class ONNXModule<Impl>;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<std::vector<float>> getOutputData(std::shared_ptr<uint8_t> buff) const;

	int module_version_;
};

template <typename Impl, TypeCrop typeCrop>
EmotionsEstimationInference<Impl, typeCrop>::EmotionsEstimationInference(const tdv::data::Context& config) :
	BaseEstimationInference<Impl, typeCrop>(config)
{};

template <typename Impl, TypeCrop typeCrop>
std::vector<std::vector<float>> EmotionsEstimationInference<Impl, typeCrop>::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	std::vector<std::vector<float>> output;
	const auto& shapes = this->getOutputShapes();
	RHAssert2(0x7b64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());
	output.emplace_back(blob_data, blob_data + predict_shape);
	return output;
}

static const std::vector<std::string> class_name = {"ANGRY", "DISGUSTED", "SCARED", "HAPPY", "NEUTRAL", "SAD", "SURPRISED"};

template <typename Impl, TypeCrop typeCrop>
void EmotionsEstimationInference<Impl, typeCrop>::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<std::vector<float>> predict = getOutputData(buffer);
		tdv::data::Context& objects = data["objects"];

		if(objects.size())
		{
			Context& obj = objects[data["objects@current_id"].get<int>()];
			for(size_t i = 0; i < predict.size(); ++i)
			{
				std::vector<double> softmax_predict = softmaxFunction(predict[i]);
				for(size_t j = 0; j < softmax_predict.size(); ++j)
				{
					Context emotion;
					emotion["emotion"] = class_name[j];
					emotion["confidence"] = softmax_predict[j];
					obj["emotions"].push_back(std::move(emotion));
				}
			}
		}
		else
		{
			objects.clear();
			for(size_t i = 0; i < predict.size(); ++i)
			{
				std::vector<double> softmax_predict = softmaxFunction(predict[i]);
				tdv::data::Context face;
				face["id"] = static_cast<int64_t>(i);
				face["class"] = "face";
				for(size_t j = 0; j < softmax_predict.size(); ++j)
				{
					Context emo;
					emo["emotion"] = class_name[j];
					emo["confidence"] = softmax_predict[j];
					face["emotions"].push_back(std::move(emo));
				}
				objects.push_back(std::move(face));
			}
		}
	}
}

}
}

#endif // EMOTIONS_ESTIMATION_INFERENCE_H