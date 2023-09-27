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
		bsmCtx["shape"].push_back(static_cast<int64_t>(img.size[i]));
	bsmCtx["shape"].push_back(static_cast<int64_t>(img.channels()));
}

cv::Mat bsmToCvMat(const Context& bsmCtx, bool copy)
{
	auto buff = bsmCtx.at("blob").get<std::shared_ptr<unsigned char>>();
	int type = StrToCvType.at(bsmCtx.at("dtype").get<std::string>());
	int ndims = static_cast<int>(bsmCtx.at("shape").size());
	std::vector<int> dims;
	for(const auto& dim : bsmCtx.at("shape"))
		dims.push_back(static_cast<int>(dim.get<int64_t>()));

	cv::Mat img(ndims-1, dims.data(), CV_MAKETYPE(type,dims.back()), buff.get());
	return copy ? img.clone() : img;
}

void keypointsBasedCrop(cv::Mat& image, const Context& data) {
	const tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];
	double left_eye_x = obj["left_eye"]["proj"][0].get<double>() * image.cols;
	double left_eye_y = obj["left_eye"]["proj"][1].get<double>() * image.rows;

	double right_eye_x = obj["right_eye"]["proj"][0].get<double>() * image.cols;
	double right_eye_y = obj["right_eye"]["proj"][1].get<double>() * image.rows;

	double ux = (left_eye_x + right_eye_x) / 2;
	double uy = (left_eye_y + right_eye_y) / 2;

	double dx = obj["mouth"]["proj"][0].get<double>() * image.cols;
	double dy = obj["mouth"]["proj"][1].get<double>() * image.rows;

	double vx = dx - ux;
	double vy = dy - uy;

	double ddy = dy + vy * 0.55;

	double uuy = uy - vy * 0.9;

	double vex = right_eye_x - left_eye_x;

	double sx = left_eye_x - vex * 0.55;
	double fx = right_eye_x + vex * 0.55;

	double box_w = fx - sx;
	double box_h = ddy - uuy;

	if (box_w > box_h)
	{
		double d = (box_w - box_h) / 2;
		ddy += d;
		uuy -= d;
	}
	else
	{
		double d = (box_h - box_w) / 2;
		fx += d;
		sx -= d;
	}

	double crop_x = sx;
	double crop_y = uuy;
	double crop_width = fx - sx;
	double crop_height = ddy - uuy;

	crop_x = std::max(0.0, crop_x);
	crop_y = std::max(0.0, crop_y);

	if(crop_x + crop_width >= image.cols - 1)
		crop_width = image.cols - 2 - crop_x;
	if(crop_y + crop_height >= image.rows - 1)
		crop_height = image.rows - 2 - crop_y;

	cv::Rect crop_rect(crop_x, crop_y, crop_width, crop_height);

	image = image(crop_rect);

}

} // namespace utils
} // namespace tdv
