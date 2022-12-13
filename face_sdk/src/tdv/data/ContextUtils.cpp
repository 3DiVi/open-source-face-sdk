#include <opencv2/core/cvdef.h>

#include <tdv/data/ContextUtils.h>
#include <iostream>
namespace tdv
{
namespace data
{

static const std::map<std::string, int> StrToCvType{{"uint8_t", CV_8U}, {"int8_t", CV_8S}, {"uint16_t", CV_16U}, {"int16_t", CV_16S},
													{"int32_t", CV_32S}, {"float", CV_32F}, {"double", CV_64F}};

static const std::map<int, std::string> CvTypeToStr{{CV_8U,"uint8_t"}, {CV_8S,"int8_t"}, {CV_16U,"uint16_t"}, {CV_16S,"int16_t"},
													{CV_32S,"int32_t"}, {CV_32F,"float"}, {CV_64F,"double"}};

void cvMatToBsm(Context& bsmCtx, const cv::Mat& img, bool copy)
{
	const bool isContinuous = img.isContinuous();

	bsmCtx["format"] = "NDARRAY";
	if (copy || !isContinuous)
	{
		size_t sizeInBytes = img.total()*img.elemSize();
		unsigned char* data = static_cast<unsigned char*>(malloc(sizeInBytes));
		if(data)
		{
			if(isContinuous)
				std::memcpy(data, img.data, sizeInBytes);
			else
			{
				unsigned char* line_ptr = data;
				size_t step = img.cols * img.elemSize();
				for(int i = 0; i < img.rows; ++i)
				{
					std::memcpy(line_ptr, img.ptr(i), step);
					line_ptr += step;
				}
			}
		}
		else
			throw std::bad_alloc();
		bsmCtx["blob"] = std::shared_ptr<unsigned char>(data, [](unsigned char* ptr){ free(ptr);});
	}
	else
		bsmCtx["blob"] = std::shared_ptr<unsigned char>(img.data, [](unsigned char*){});


	bsmCtx["dtype"] = CvTypeToStr.at(img.depth());
	bsmCtx.erase("shape");
	for(int i = 0; i < img.dims; ++i)
		bsmCtx["shape"].push_back(static_cast<long>(img.size[i]));
	bsmCtx["shape"].push_back(static_cast<long>(img.channels()));
}

cv::Mat bsmToCvMat(const Context& bsmCtx, bool copy)
{
	auto buff = bsmCtx.at("blob").get<std::shared_ptr<unsigned char>>();
	int type = StrToCvType.at(bsmCtx.at("dtype").get<std::string>());
	int ndims = static_cast<int>(bsmCtx.at("shape").size());
	std::vector<int> dims;
	for(const auto& dim : bsmCtx.at("shape"))
		dims.push_back(static_cast<int>(dim.get<long>()));

	cv::Mat img(ndims-1, dims.data(), CV_MAKETYPE(type,dims.back()), buff.get());
	return copy ? img.clone() : img;
}

cv::Mat keypointsBasedCrop(Context& data, cv::Mat& image) {

	float left_eye_x = data["objects"][0]["fitter"]["left_eye"][0].get<double>() * image.cols;
	float left_eye_y = data["objects"][0]["fitter"]["left_eye"][1].get<double>() * image.rows;

	float right_eye_x = data["objects"][0]["fitter"]["right_eye"][0].get<double>() * image.cols;
	float right_eye_y = data["objects"][0]["fitter"]["right_eye"][1].get<double>() * image.rows;

	float ux = (left_eye_x + right_eye_x) / 2;
	float uy = (left_eye_y + right_eye_y) / 2;

	float dx = data["objects"][0]["fitter"]["mouth"][0].get<double>() * image.cols;
	float dy = data["objects"][0]["fitter"]["mouth"][1].get<double>() * image.rows;

	float vx = dx - ux;
	float vy = dy - uy;

	float ddx = dx + vx * 0.9;
	float ddy = dy + vy * 0.55;

	float uux = ux - vx * 1.5;
	float uuy = uy - vy * 0.9;

	float vex = right_eye_x - left_eye_x;

	float sx = left_eye_x - vex * 0.55;
	float fx = right_eye_x + vex * 0.55;

	float box_w = fx - sx;
	float box_h = ddy - uuy;

	if (box_w > box_h)
	{
		float d = (box_w - box_h) / 2;
		ddy += d;
		uuy -= d;
	}
	else
	{
		float d = (box_h - box_w) / 2;
		fx += d;
		sx -= d;
	}

	float crop_x = sx;
	float crop_y = uuy;
	float crop_width = fx - sx;
	float crop_height = ddy - uuy;

	crop_x = std::max(0.0f, crop_x);
	crop_y = std::max(0.0f, crop_y);

	if(crop_x + crop_width >= image.cols - 1)
		crop_width = image.cols - 2 - crop_x;
	if(crop_y + crop_height >= image.rows - 1)
		crop_height = image.rows - 2 - crop_y;

	cv::Rect crop_rect(crop_x, crop_y, crop_width, crop_height);

	cv::Mat crop_image = image(crop_rect);

	return crop_image;

}

cv::Mat keypointsBasedCropOneObject(const Context& data, cv::Mat& image) {

	float left_eye_x = data["fitter"]["left_eye"][0].get<double>() * image.cols;
	float left_eye_y = data["fitter"]["left_eye"][1].get<double>() * image.rows;

	float right_eye_x = data["fitter"]["right_eye"][0].get<double>() * image.cols;
	float right_eye_y = data["fitter"]["right_eye"][1].get<double>() * image.rows;

	float ux = (left_eye_x + right_eye_x) / 2;
	float uy = (left_eye_y + right_eye_y) / 2;

	float dx = data["fitter"]["mouth"][0].get<double>() * image.cols;
	float dy = data["fitter"]["mouth"][1].get<double>() * image.rows;

	float vx = dx - ux;
	float vy = dy - uy;

	float ddx = dx + vx * 0.9;
	float ddy = dy + vy * 0.55;

	float uux = ux - vx * 1.5;
	float uuy = uy - vy * 0.9;

	float vex = right_eye_x - left_eye_x;

	float sx = left_eye_x - vex * 0.55;
	float fx = right_eye_x + vex * 0.55;

	float box_w = fx - sx;
	float box_h = ddy - uuy;

	if (box_w > box_h)
	{
		float d = (box_w - box_h) / 2;
		ddy += d;
		uuy -= d;
	}
	else
	{
		float d = (box_h - box_w) / 2;
		fx += d;
		sx -= d;
	}

	float crop_x = sx;
	float crop_y = uuy;
	float crop_width = fx - sx;
	float crop_height = ddy - uuy;

	crop_x = std::max(0.0f, crop_x);
	crop_y = std::max(0.0f, crop_y);

	if(crop_x + crop_width >= image.cols - 1)
		crop_width = image.cols - 2 - crop_x;
	if(crop_y + crop_height >= image.rows - 1)
		crop_height = image.rows - 2 - crop_y;

	cv::Rect crop_rect(crop_x, crop_y, crop_width, crop_height);

	cv::Mat crop_image = image(crop_rect);

	return crop_image;

}

} // namespace utils
} // namespace tdv
