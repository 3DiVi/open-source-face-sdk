#ifndef BASE_ESTIMATION_INFERENCE_H
#define BASE_ESTIMATION_INFERENCE_H

#include <new>
#include <tuple>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <tdv/data/ContextUtils.h>
#include <tdv/modules/ONNXModule.h>
#include <tdv/utils/rassert/RAssert.h>

#undef max
#undef min

namespace{
template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

void parseRoiBoxFromBboxAndGetCrop(cv::Mat &image, tdv::data::Context &data){
	tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];
	tdv::data::Context& rectCtx = obj["bbox"];
	double left = rectCtx[0].get<double>() * image.cols;
	double top = rectCtx[1].get<double>() * image.rows;
	double right = rectCtx[2].get<double>() * image.cols;
	double bottom = rectCtx[3].get<double>() * image.rows;

	double old_size = (right - left + bottom - top) / 2;
	double center_x = right - (right - left) / 2.0;
	double center_y = bottom - (bottom - top) / 2.0 + old_size * 0.14;
	int size = int(old_size * 1.58);


	tdv::data::Context& roiBox = obj["roi_bbox"];
	roiBox.push_back((center_x - size / 2) /  image.cols);
	roiBox.push_back((center_y - size / 2) /  image.rows);
	roiBox.push_back((center_x + size / 2) /  image.cols);
	roiBox.push_back((center_y + size / 2) /  image.rows);

	int sx = (int)round(center_x - size / 2);
	int sy = (int)round(center_y - size / 2);
	int ex = (int)round(center_x + size / 2);
	int ey = (int)round(center_y + size / 2);

	int dh = ey - sy, dw = ex - sx;
	int dsx = 0, dsy = 0, dex = dw, dey = dh;

	if (sx < 0){
		dsx = -sx;
		sx = 0;
	}

	if (ex > image.cols){
		dex = dw - (ex - image.cols);
		ex = image.cols;
	}

	 if (sy < 0){
		dsy = -sy;
		sy = 0;
	 }

	 if (ey > image.rows){
		dey = dh - (ey - image.rows);
		ey = image.rows;
	 }

	cv::Mat res(dh, dw, (image.channels() == 3 ? CV_8UC3 : CV_8UC1), cv::Scalar(0));
	image(cv::Rect(cv::Point{sx, sy}, cv::Point{ex, ey})).copyTo(res(cv::Rect(cv::Point{dsx, dsy}, cv::Point{dex, dey})));

	image = std::move(res);
}

void getSimpleCrop(cv::Mat &image, const tdv::data::Context &data){
	const tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];

	const tdv::data::Context& rectCtx = obj["bbox"];
	cv::Point bbox_top_left = {clip(static_cast<int>(rectCtx[0].get<double>() * image.cols), 0, image.cols), clip(static_cast<int>(rectCtx[1].get<double>() * image.rows), 0 , image.rows)}; //TODO add border of image
	cv::Point bbox_bottom_right = {clip(static_cast<int>(rectCtx[2].get<double>() * image.cols), 0, image.cols), clip(static_cast<int>(rectCtx[3].get<double>() * image.rows), 0 , image.rows)}; //TODO add border of image
	image = image(cv::Rect(bbox_top_left,bbox_bottom_right));
}

}

namespace tdv {

namespace modules {

enum TypeCrop
{
	SIMPLE_CROP = 0,
	KEYPOINTS_BASED_CROP = 1,
	FDA_ROI_CROP = 2
};


template <typename Impl, TypeCrop typeCrop = SIMPLE_CROP>
class BaseEstimationInference : public ONNXModule<Impl>
{
public:
	BaseEstimationInference(const tdv::data::Context& config);
private:
	friend class ONNXModule<Impl>;
	void virtual preprocess(tdv::data::Context& data) override;

protected:
	cv::Mat virtual blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F);
	int module_version_;

	bool isNormaliseImage = true;
	std::vector<double> mean = {0.5, 0.5, 0.5};
	std::vector<double> std = {0.5, 0.5, 0.5};

	int nchannel_index = 1;
	int input_size_index = 2;
};


template <typename Impl, TypeCrop typeCrop>
BaseEstimationInference<Impl, typeCrop>::BaseEstimationInference(const tdv::data::Context& config):
	ONNXModule<Impl>(config)
{
	module_version_ = config.get<int64_t>("model_version", 1);
}

template <typename Impl, TypeCrop typeCrop>
void BaseEstimationInference<Impl, typeCrop>::preprocess(tdv::data::Context& data) {

	Context& firstInput = data["image"];
	cv::Mat image = tdv::data::bsmToCvMat(firstInput, true);

	if (data.contains("objects")){
		if (typeCrop == SIMPLE_CROP){
			getSimpleCrop(image, data);
		}else if(typeCrop == KEYPOINTS_BASED_CROP){
			tdv::data::keypointsBasedCrop(image, data);
		}else if (typeCrop == FDA_ROI_CROP){
			parseRoiBoxFromBboxAndGetCrop(image, data);
		}
	}

	RHAssert2(0x7a11d233,  image.depth() == CV_8U ||  image.depth() == CV_32F, "only 8U and 32F image types are suported");

	const auto& shape = this->getInputShapes();
	const auto& INPUT_SIZE = shape.front()[input_size_index];
	const auto& N_CHANNEL = shape.front()[nchannel_index];

	cv::resize(image, image, cv::Size(INPUT_SIZE, INPUT_SIZE), 0, 0);

	size_t sizeInBytes = INPUT_SIZE * INPUT_SIZE * N_CHANNEL * sizeof(float);
	unsigned char* input_ptr = static_cast<unsigned char*>(malloc(sizeInBytes));

	if(!input_ptr)
		throw std::bad_alloc();
	cv::Mat img_blob = blobFromImage(image, N_CHANNEL);

	memcpy(input_ptr, img_blob.data, sizeInBytes);

	Context& inputData = data["objects@input"][0];
	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr, [](unsigned char* ptr){ free(ptr);});
}

template <typename Impl, TypeCrop typeCrop>
cv::Mat BaseEstimationInference<Impl, typeCrop>::blobFromImage(cv::Mat& image, int nchannel, const int ddepth) {

	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);

	if(image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth, 1.0f/255);
	}

	int nch = image.channels();
	int sz[] = { 1, nchannel, image.rows, image.cols};

	output.create(4, sz, ddepth);

	std::vector<cv::Mat> ch(nch);

	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));

	cv::split(image, ch);

	//Normalize image
	RHAssert2(0x11561384, nchannel == 3 && nch == 3, "Need 1 or 3 channels image (Gray or RGB)");

	if (isNormaliseImage)
	{
		for(int i=0; i < nchannel; i++)
			ch[i] = (ch[i] - mean[i]) / std[i];
	}

	return output;
}

}
}


#endif // BASE_ESTIMATION_INFERENCE_H
