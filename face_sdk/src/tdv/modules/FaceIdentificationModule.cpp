#include <new>
#include <tuple>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/utils/recognizer_utils/RecognizerUtils.h>
#include <tdv/modules/FaceIdentificationModule.h>
#include <tdv/utils/rassert/RAssert.h>



namespace {

using namespace tdv::utils::recognizer_utils;

cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) {

	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);

	if(image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth);
	}

	int nch = image.channels();
	int sz[] = { nchannel, image.rows, image.cols};

	output.create(3, sz, ddepth);

	cv::Mat* ch = new cv::Mat[nch];
	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(j));

	cv::split(image, ch);

	return output;
}


cv::Mat processObject(const tdv::data::Context& obj, const int input_width, const int input_height){
	std::shared_ptr<cv::Mat> image = obj.at("object@image").get<std::shared_ptr<cv::Mat>>();

	int img_width = (*image).cols;
	int img_height = (*image).rows;

	if (obj.contains("fitter")){
		const cv::Matx23f crop2image = makeCrop2ImageByPoints(obj["fitter"], (*image), (std::max)(input_width, input_height));
		cv::Mat result;
		warpAffine((*image), result, crop2image, cv::Size(input_width, input_height));
		return result;
	}

	const tdv::data::Context& rectCtx = obj["bbox"];  // const overload calls .at()
	cv::Rect rect(cv::Point{static_cast<int>(rectCtx[0].get<double>()*img_width), static_cast<int>(rectCtx[1].get<double>()*img_height)},
				  cv::Point{static_cast<int>(rectCtx[2].get<double>()*img_width), static_cast<int>(rectCtx[3].get<double>()*img_height)});

	return (*image)(rect);
}

void imageToInput(cv::Mat &image, tdv::data::Context& data, size_t sizeInBytes, int N_CHANNEL){
	unsigned char* input_ptr = static_cast<unsigned char*>(malloc(sizeInBytes));
	if(!input_ptr)
		throw std::bad_alloc();
	cv::Mat img_blob = blobFromImage(image, N_CHANNEL);

	memcpy(input_ptr, img_blob.data, sizeInBytes);

	tdv::data::Context& inputData = data["objects@input"][0];
	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr, [](unsigned char* ptr){ free(ptr);});
}

void l2Normalize(std::vector<float> &input_output){

	float norm2 = 0;
	for (int i = 0; i < input_output.size(); i++)
		norm2 += input_output[i] * input_output[i];

	if(norm2 > 0)
	{
		const float m = 1.0 / sqrt(norm2);
		for(size_t i = 0; i < input_output.size(); ++i)
			input_output[i] *= m;
	}

}

}

namespace tdv {

namespace modules {


FaceIdentificationModule::FaceIdentificationModule(const tdv::data::Context& config) :
		ONNXModule<FaceIdentificationModule>(config)
{};

std::vector<float> FaceIdentificationModule::getOutputData(std::shared_ptr<uint8_t> buff)
{
	const auto& shapes = getOutputShapes();
	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());
	std::vector<std::vector<float>> result;

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};
	l2Normalize(result_predict);

	return result_predict;
}

void FaceIdentificationModule::preprocess(tdv::data::Context& data) {
	const auto& shape = this->getInputShapes();
	const auto& INPUT_H = shape.front()[2];
	const auto& INPUT_W = shape.front()[3];
	const auto& N_CHANNEL = shape.front()[1];

	size_t sizeInBytesOne = INPUT_W * INPUT_H * N_CHANNEL * sizeof(float);

	cv::Mat image;
	if (data.contains("class")){
		RHAssert2(0x324b3157, data.get<std::string>("class", "") == "face", "need class face");
		image = processObject(data, INPUT_W, INPUT_H);
	}else{
		Context& imageInput = data.at("image");
		image = tdv::data::bsmToCvMat(imageInput, true);
		RHAssert2(0x11113333, image.depth() == CV_8U || image.depth() == CV_32F, "only 8U and 32F image types are suported");
	}
	cv::resize(image, image, cv::Size(INPUT_W, INPUT_H));
	imageToInput(image, data, sizeInBytesOne, N_CHANNEL);
}

void FaceIdentificationModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> embeds = getOutputData(buffer);

		if (data.contains("class")){
			data["template_size"] = (long)embeds.size();
			data["template"] = std::move(embeds);
		}else{
			tdv::data::Context obj;
			obj["class"] = "face";
			obj["template_size"] = (long)embeds.size();
			obj["template"] = std::move(embeds);
			data["objects"].push_back(std::move(obj));
		}
	}
}

}
}
