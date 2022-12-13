#include <new>
#include <cmath>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/MeshFitterModule.h>
#include <tdv/utils/rassert/RAssert.h>

namespace {

cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) {

	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);

	if(image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth, 1.f/255);
	}

	int nch = image.channels();
	int sz[] = { 1, nchannel, image.rows, image.cols};

	output.create(4, sz, ddepth);

	cv::Mat* ch = new cv::Mat[nch];
	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));


	cv::split(image, ch);

	return output;
}

cv::Mat processObject(const tdv::data::Context& obj){
	std::shared_ptr<cv::Mat> image = obj.at("object@image").get<std::shared_ptr<cv::Mat>>();

	int img_width = (*image).cols;
	int img_height = (*image).rows;

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

void getSpecialPoint(std::string key, tdv::data::Context& context, std::vector<int> &point_indexs){
	double x = 0, y = 0, z = 0;
	for (const int& index : point_indexs){
		x += context["keypoints"][index][0].get<double>();
		y += context["keypoints"][index][1].get<double>();
		z += context["keypoints"][index][2].get<double>();
	}
	context[key].push_back(x / point_indexs.size());
	context[key].push_back(y / point_indexs.size());
	context[key].push_back(z / point_indexs.size());
}
}


namespace tdv {

namespace modules {


MeshFitterModule::MeshFitterModule(const tdv::data::Context& config) :
	ONNXModule<MeshFitterModule>(config)
	{
		const auto& shape = getInputShapes();
		RHAssert2(0x8758bc91, shape.front()[2] == shape.front()[3], "incorrect input shape");
	};

std::vector<float> MeshFitterModule::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1] + 1)};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}

void MeshFitterModule::preprocess(tdv::data::Context& data) {
	const auto& shape = this->getInputShapes();
	const auto& INPUT_H = shape.front()[2];
	const auto& INPUT_W = shape.front()[3];
	const auto& N_CHANNEL = shape.front()[1];

	size_t sizeInBytesOne = INPUT_W * INPUT_H * N_CHANNEL * sizeof(float);
	cv::Mat image;
	if (data.contains("class")){
		RHAssert2(0x324b3157, data.get<std::string>("class", "") == "face", "need class face");
		image = processObject(data);
	}else{
		Context& imageInput = data.at("image");
		image = tdv::data::bsmToCvMat(imageInput, true);
		RHAssert2(0x11113333, image.depth() == CV_8U || image.depth() == CV_32F, "only 8U and 32F image types are suported");
	}
	cv::resize(image, image, cv::Size(INPUT_W, INPUT_H));
	imageToInput(image, data, sizeInBytesOne, N_CHANNEL);
}

void MeshFitterModule::objectFromPredict(std::vector<float> &predict, tdv::data::Context& obj, const int INPUT_SIZE){
	tdv::data::Context& key_points = obj["fitter"]["keypoints"];
	int i_w = 1;
	int i_h = 1;
	double o_x = 0;
	double o_y = 0;
	double ci_w = 1;
	double ci_h = 1;

	if (obj.contains("bbox")){
		std::shared_ptr<cv::Mat> image = obj.at("object@image").get<std::shared_ptr<cv::Mat>>();
		i_w = (*image).cols;
		i_h = (*image).rows;
		o_x = obj["bbox"][0].get<double>();
		o_y = obj["bbox"][1].get<double>();
		ci_w = obj["bbox"][2].get<double>() * i_w - o_x * i_w;
		ci_h = obj["bbox"][3].get<double>() * i_h - o_y * i_h ;
	}

	for (int i = 0; i < predict.size() - 1; i += 3){
		tdv::data::Context point;
		point.push_back(static_cast<double>(o_x + (predict[i] / INPUT_SIZE) * (ci_w / i_w)));
		point.push_back(static_cast<double>(o_y + (predict[i + 1] / INPUT_SIZE) * (ci_h / i_h)));
		point.push_back(static_cast<double>(predict[i + 2] / INPUT_SIZE));
		key_points.push_back(std::move(point));
	}

	obj["fitter"]["score"] = static_cast<double>(predict[1404] / 50);

	getSpecialPoint("left_eye", obj["fitter"], l_idx);
	getSpecialPoint("right_eye", obj["fitter"], r_idx);
	getSpecialPoint("mouth", obj["fitter"], mouth_idx);

	obj["fitter"]["fitter_type"] = "mesh";
}

void MeshFitterModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> embeds = getOutputData(buffer);
		const auto INPUT_SIZE = getInputShapes().front()[2];

		if (data.contains("class")){
			objectFromPredict(embeds, data, INPUT_SIZE);
		}else{
			tdv::data::Context obj;
			obj["class"] = "face";
			objectFromPredict(embeds, obj, INPUT_SIZE);
			data["objects"].push_back(std::move(obj));
		}
	}
}

}
}
