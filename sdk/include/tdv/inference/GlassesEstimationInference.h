#ifndef GLASSES_ESTIMATION_INFERENCE_H
#define GLASSES_ESTIMATION_INFERENCE_H

#include <tdv/inference/BaseEstimationInference.h>


namespace tdv {

namespace modules {

template <typename Impl, TypeCrop typeCrop = SIMPLE_CROP>
class GlassesEstimationInference : public BaseEstimationInference<Impl, typeCrop>
{
public:
	GlassesEstimationInference(const tdv::data::Context& config);
private:
	friend class ONNXModule<Impl>;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;

	int module_version_;
	const double GLASSES_THRESH;
//protected:
	cv::Mat virtual blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) override;
};


template <typename Impl, TypeCrop typeCrop>
GlassesEstimationInference<Impl, typeCrop>::GlassesEstimationInference(const tdv::data::Context& config):
	BaseEstimationInference<Impl, typeCrop>(config), GLASSES_THRESH(config.get<double>("glasses_threshold", 0.992))
{
	this->nchannel_index = 3;
}


template <typename Impl, TypeCrop typeCrop>
std::vector<float> GlassesEstimationInference<Impl, typeCrop>::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = this->getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}


template <typename Impl, TypeCrop typeCrop>
void GlassesEstimationInference<Impl, typeCrop>::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data)
{
	if(buffer)
	{
		std::vector<float> predict = getOutputData(buffer);
		tdv::data::Context& objects = data["objects"];
		if(objects.size())
		{
			Context& obj = objects[data["objects@current_id"].get<int>()];
			for(size_t i = 0; i < predict.size(); ++i)
			{
				obj["has_glasses"] = GLASSES_THRESH < static_cast<double>(predict[i]);
				obj["glasses_confidence"] = static_cast<double>(predict[i]);
			}
		}
		else
		{
			objects.clear();
			for(size_t i = 0; i < predict.size(); ++i)
			{
				tdv::data::Context face;
				face["id"] = static_cast<int64_t>(i);
				face["class"] = "face";
				face["has_glasses"] = GLASSES_THRESH < static_cast<double>(predict[i]);
				face["glasses_confidence"] = static_cast<double>(predict[i]);
				objects.push_back(std::move(face));
			}
		}
	}
}

template <typename Impl, TypeCrop typeCrop>
cv::Mat GlassesEstimationInference<Impl, typeCrop>::blobFromImage(cv::Mat& image, int nchannel, const int ddepth)
{
	cv::Mat output;
	if(image.channels() == 3)
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);

	if(image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth, 1.0f/255);
	}
	cv::merge(std::vector<cv::Mat>(3, image), image);
	int nch = image.channels();
	int sz[] = { 1, nchannel, image.rows, image.cols};

	output.create(4, sz, ddepth);

	std::vector<cv::Mat> ch(nch);

	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));

	cv::split(image, ch);

	//Normalize image
	RHAssert2(0x11561384, nchannel == 3 && nch == 3, "Need 1 or 3 channels image (Gray or RGB)");

	for(int i=0; i < nchannel; i++)
	{
		cv::Mat mean;
		cv::Mat std;
		cv::meanStdDev(ch[i], mean, std);
		ch[i] = (ch[i] - mean.at<double>(0)) / std.at<double>(0);
	}

	return output;
}


}
}


#endif // GLASSES_ESTIMATION_INFERENCE_H
