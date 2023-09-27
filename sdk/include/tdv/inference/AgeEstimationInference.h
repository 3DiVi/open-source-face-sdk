#ifndef AGE_ESTIMATION_INFERENCE_H
#define AGE_ESTIMATION_INFERENCE_H

#include <tdv/inference/BaseEstimationInference.h>


namespace tdv {

namespace modules {

template <typename Impl, TypeCrop typeCrop = SIMPLE_CROP>
class AgeEstimationInference : public BaseEstimationInference<Impl, typeCrop>
{
public:
	AgeEstimationInference(const tdv::data::Context& config);
private:
	friend class ONNXModule<Impl>;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;

	int module_version_;
};


template <typename Impl, TypeCrop typeCrop>
AgeEstimationInference<Impl, typeCrop>::AgeEstimationInference(const tdv::data::Context& config):
	BaseEstimationInference<Impl, typeCrop>(config)
{}


template <typename Impl, TypeCrop typeCrop>
std::vector<float> AgeEstimationInference<Impl, typeCrop>::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = this->getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}


template <typename Impl, TypeCrop typeCrop>
void AgeEstimationInference<Impl, typeCrop>::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> predict = getOutputData(buffer);
		tdv::data::Context& objects = data["objects"];
		if(objects.size())
		{
			Context& obj = objects[data["objects@current_id"].get<int>()];
			for(size_t i = 0; i < predict.size(); ++i)
				obj["age"] = static_cast<int64_t>(round(predict[i]));
		}
		else
		{
			objects.clear();
			for(size_t i = 0; i < predict.size(); ++i)
			{
				tdv::data::Context face;
				face["id"] = static_cast<int64_t>(i);
				face["class"] = "face";
				face["age"] = static_cast<int64_t>(round(predict[i]));
				objects.push_back(std::move(face));
			}
		}
	}
}


}
}


#endif // AGEESTIMATION_H
