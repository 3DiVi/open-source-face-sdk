#include <new>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/utils/recognizer_utils/RecognizerUtils.h>
#include <tdv/modules/FaceIdentificationModule.h>
#include <tdv/utils/rassert/RAssert.h>

namespace {

using namespace tdv::utils::recognizer_utils;


template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max<T>(lower, std::min<T>(n, upper));
}

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

	std::vector<cv::Mat> ch(nch);

	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(j));

	cv::split(image, ch);

	return output;
}


void processObject(cv::Mat &image, const tdv::data::Context data, const int input_width, const int input_height){
	const tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];

	int img_width = data["image"]["shape"][1].get<int64_t>();
	int img_height = data["image"]["shape"][0].get<int64_t>();

	if (obj.contains("keypoints")){
		const cv::Matx23f crop2image = makeCrop2ImageByPoints(obj["keypoints"], image, (std::max)(input_width, input_height));
		warpAffine(image, image, crop2image, cv::Size(input_width, input_height));
	}else{
		const tdv::data::Context& rectCtx = obj["bbox"];
		cv::Point bbox_top_left = {clip(static_cast<int>(rectCtx[0].get<double>() * image.cols), 0, image.cols), clip(static_cast<int>(rectCtx[1].get<double>() * image.rows), 0 , image.rows)}; //TODO add border of image
		cv::Point bbox_bottom_right = {clip(static_cast<int>(rectCtx[2].get<double>() * image.cols), 0, image.cols), clip(static_cast<int>(rectCtx[3].get<double>() * image.rows), 0 , image.rows)}; //TODO add border of image
		image = image(cv::Rect(bbox_top_left,bbox_bottom_right));
	}
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

	Context& firstInput = data["image"];
	cv::Mat image = tdv::data::bsmToCvMat(firstInput, true);

	const auto& shape = this->getInputShapes();
	const auto& INPUT_H = shape.front()[2];
	const auto& INPUT_W = shape.front()[3];
	const auto& N_CHANNEL = shape.front()[1];

	if (data.contains("objects")){
		processObject(image, data, INPUT_W, INPUT_H);
	}

	RHAssert2(0x11113333, image.depth() == CV_8U || image.depth() == CV_32F, "only 8U and 32F image types are suported");

	cv::resize(image, image, cv::Size(INPUT_W, INPUT_H));

	size_t sizeInBytes = INPUT_W * INPUT_H * N_CHANNEL * sizeof(float);
	unsigned char* input_ptr = static_cast<unsigned char*>(malloc(sizeInBytes));
	if(!input_ptr)
		throw std::bad_alloc();
	cv::Mat img_blob = blobFromImage(image, N_CHANNEL);

	memcpy(input_ptr, img_blob.data, sizeInBytes);

	tdv::data::Context& inputData = data["objects@input"][0];
	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr, [](unsigned char* ptr){ free(ptr);});
}

void FaceIdentificationModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> embeds = getOutputData(buffer);

		tdv::data::Context& objects = data["objects"];
		tdv::data::Context& imgShape = data["image"]["shape"];
		if(objects.size())
		{
			Context& obj = objects[data["objects@current_id"].get<int>()];
			obj["template_size"] = (int64_t)embeds.size();
			obj["template"] = std::move(embeds);
		}
		else
		{
			objects.clear();
			tdv::data::Context face;
			face["id"] = 0l;
			face["class"] = "face";
			face["template_size"] = (int64_t)embeds.size();
			face["template"] = std::move(embeds);
			objects.push_back(std::move(face));
		}
	}
}

void FaceIdentificationModule::operator ()(tdv::data::Context& data){
	if (data.contains("objects")){
		for(int i = 0; i < data["objects"].size(); i++){
			data["objects@current_id"] = i;
			ONNXModule<FaceIdentificationModule>::operator ()(data);
		}
		data.erase("objects@current_id");
	}else{
		ONNXModule<FaceIdentificationModule>::operator ()(data);
	}
}

}
}
