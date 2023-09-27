#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/LivenessDetectionModule/LivenessBaseModule.h>
#include <tdv/utils/rassert/RAssert.h>
#include <math.h>


namespace {
cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) 
{
	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
	image.convertTo(image, CV_32FC3);

	int nch = image.channels();
	int sz[] = { 1, nchannel, image.rows, image.cols};

	output.create(4, sz, ddepth);

	std::vector<cv::Mat> ch(nch);
	
	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));
	cv::split(image, ch);

	RHAssert2(0x11561385, nchannel == 3 && nch == 3, "Need 3 channel image (RGB)");
	return output;
}

std::vector<float> softmax(const std::vector<float> data)
{
	int i;
	double m, sum, constant;
	m = -INFINITY;
	for (i = 0; i < data.size(); ++i) {
		if (m < data.at(i))
			m = data.at(i);
	}
	std::vector<float> result(data.size());

	sum = 0.0;
	for (i = 0; i < data.size(); ++i)
		sum += exp(data.at(i) - m);

	constant = m + log(sum);
	for (i = 0; i < data.size(); ++i)
		result[i] = exp(data.at(i) - constant);

	return result;
}
}

namespace tdv {
namespace modules {


LivenessBaseModule::LivenessBaseModule(const tdv::data::Context& config):
	ONNXModule<LivenessBaseModule>(config)
{}

std::vector<float> LivenessBaseModule::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}

void LivenessBaseModule::preprocess(tdv::data::Context& data) {

	Context& firstInput = data["image"];
	cv::Mat image = tdv::data::bsmToCvMat(firstInput, true);

	RHAssert2(0x7a11d233,  image.depth() == CV_8U ||  image.depth() == CV_32F, "only 8U and 32F image types are suported");

	const auto& shape = getInputShapes();
	const auto& INPUT_SIZE = shape.front()[2];
	const auto& N_CHANNEL = shape.front()[1];

	cv::resize(image, image, cv::Size(INPUT_SIZE, INPUT_SIZE), 0, 0);

	size_t sizeInBytes = INPUT_SIZE * INPUT_SIZE * N_CHANNEL * sizeof(float);
	unsigned char* input_ptr = static_cast<unsigned char*>(malloc(sizeInBytes));

	if(!input_ptr)
		throw std::bad_alloc();
	cv::Mat img_blob = blobFromImage(image, N_CHANNEL);

	memcpy(input_ptr, img_blob.data, sizeInBytes);

	Context& inputData = data["objects@input"][0];
	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr, [](unsigned char* ptr){ free(ptr);});
	return;
}


void LivenessBaseModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> pre_predict = getOutputData(buffer);
		std::vector<float> predict = softmax(pre_predict);
		tdv::data::Context& objects = data["objects"];
		objects.clear();
		
		tdv::data::Context face;
		face["liveness"] = static_cast<double>(predict[1]);
		objects.push_back(std::move(face));
		
	}
}

}
}