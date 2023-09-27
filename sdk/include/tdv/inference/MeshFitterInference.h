#ifndef MESH_FITTER_INFERENCE_H
#define MESH_FITTER_INFERENCE_H

#include <tdv/inference/BaseEstimationInference.h>
#include <tdv/utils/recognizer_utils/RecognizerUtils.h>


namespace tdv {

namespace modules {

template <typename Impl, TypeCrop typeCrop = SIMPLE_CROP>
class MeshFitterInference : public BaseEstimationInference<Impl, typeCrop>
{
public:
	MeshFitterInference(const tdv::data::Context& config);
private:
	friend class ONNXModule<Impl>;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;

	void objectFromPredict(std::vector<float> &predict, tdv::data::Context& obj, const int INPUT_SIZE, cv::Size2i frameSize);

	int module_version_;
};


template <typename Impl, TypeCrop typeCrop>
MeshFitterInference<Impl, typeCrop>::MeshFitterInference(const tdv::data::Context& config):
	BaseEstimationInference<Impl, typeCrop>(config)
{
	this->isNormaliseImage = false;
}


template <typename Impl, TypeCrop typeCrop>
std::vector<float> MeshFitterInference<Impl, typeCrop>::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = this->getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1] + 1)};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}

template <typename Impl, TypeCrop typeCrop>
void MeshFitterInference<Impl, typeCrop>::objectFromPredict(std::vector<float> &predict, tdv::data::Context& obj, const int INPUT_SIZE, cv::Size2i frameSize){
	tdv::data::Context& key_points = obj["keypoints"];
	int i_w = 1;
	int i_h = 1;
	double o_x = 0;
	double o_y = 0;
	double ci_w = 1;
	double ci_h = 1;

	if (obj.contains("bbox")){
		i_w = frameSize.width;
		i_h = frameSize.height;

		o_x = obj["bbox"][0].get<double>();
		o_y = obj["bbox"][1].get<double>();
		ci_w = obj["bbox"][2].get<double>() * i_w - o_x * i_w;
		ci_h = obj["bbox"][3].get<double>() * i_h - o_y * i_h ;
	}

	tdv::data::Context& points = key_points["points"];
	for (int i = 0; i < predict.size() - 1; i += 3){
		tdv::data::Context point;
		point["x"] = static_cast<double>(o_x + (predict[i] / INPUT_SIZE) * (ci_w / i_w));
		point["y"] = static_cast<double>(o_y + (predict[i + 1] / INPUT_SIZE) * (ci_h / i_h));
		point["z"] = static_cast<double>(predict[i + 2] / INPUT_SIZE);
		points.push_back(std::move(point));
	}

	key_points["fitter_type"] = "mesh";

	tdv::utils::recognizer_utils::constructFdaPonints2Context(key_points);
}


template <typename Impl, TypeCrop typeCrop>
void MeshFitterInference<Impl, typeCrop>::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> predict = getOutputData(buffer);
		const auto INPUT_SIZE = this->getInputShapes().front()[2];
		tdv::data::Context& objects = data["objects"];
		tdv::data::Context& imgShape = data["image"]["shape"];
		if(objects.size())
		{
			Context& obj = objects[data["objects@current_id"].get<int>()];
			objectFromPredict(predict, obj, INPUT_SIZE, cv::Size2i(imgShape[1].get<int64_t>(), imgShape[0].get<int64_t>()));
		}
		else
		{
			objects.clear();
			tdv::data::Context face;
			face["id"] = 0l;
			face["class"] = "face";
			objectFromPredict(predict, face, INPUT_SIZE, cv::Size2i(imgShape[1].get<int64_t>(), imgShape[0].get<int64_t>()));
			objects.push_back(std::move(face));
		}
	}
}


}
}


#endif // MESH_FITTER_INFERENCE_H
