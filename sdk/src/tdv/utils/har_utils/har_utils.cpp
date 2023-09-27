#include <tdv/data/JSONSerializer.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <map>
#include <string>


cv::Mat blobFromImage(cv::Mat &image, int nchannel = 3, const int ddepth = CV_32F)
{
	cv::Mat output;
	if (image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
	if (image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth);

	}
	int nch = image.channels();
	int sz[] = {1, nchannel, image.rows, image.cols};
	output.create(4, sz, ddepth);
	std::vector<cv::Mat> ch(nch);
	for (int j = 0; j < nchannel; j++)
	{
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0, j));
	}
	cv::split(image, ch);
	return output;
}



std::map<int, std::string> read_label_map(std::string label_path)
{
	std::map<int, std::string> label_map;
	tdv::data::Context labelMapCtx;
	std::ifstream t(label_path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	labelMapCtx = tdv::data::JSONSerializer::deserialize(buffer.str());

	for (auto itr = labelMapCtx.kvbegin(); itr != labelMapCtx.kvend(); ++itr)
	{
		int key = std::stoi(itr->first);
		std::string label = itr->second.get<std::string>();
		label_map[key] = label;
	}
	return label_map;
};

tdv::data::Context pose_vector2normalizedCtx(std::vector<std::vector<float>> &keypoints,
							   std::map<int, std::string> &label_map,
							   std::vector<int>& dims)
{
	tdv::data::Context poses;
	for (int i = 0; i < keypoints.size(); i++)
	{
		tdv::data::Context point;
		point["proj"].push_back(static_cast<double>(keypoints[i][0]/dims[1]));
		point["proj"].push_back(static_cast<double>(keypoints[i][1]/dims[0]));
		point["confidence"] = static_cast<double>(keypoints[i][2]);
		poses[label_map[i]] = std::move(point);
	}
	return poses;
}

void bboxScaler (std::vector<double> &bbox,
				  const std::vector<int> &dims,
				  double padding ){

	double aspect_ratio = static_cast<double>(dims[0]) / dims[1];
	double center_x = (bbox[0]+bbox[2])/2.0;
	double center_y = (bbox[1]+bbox[3])/2.0;
	double w = bbox[2] - bbox[0];
	double h = bbox[3] - bbox[1];

	if (w > aspect_ratio * h)
	{
		h = w * 1.0 / aspect_ratio;
	}
	else
	{
		if (w < aspect_ratio * h)
		{
			w = h * aspect_ratio;
		}
	}

	w *= padding;
	h *= padding;

	bbox = {center_x - w * 0.5,
			center_y - h * 0.5,
			center_x + w * 0.5,
			center_y + h * 0.5};
}


std::vector<double> resizeWithPad(cv::Mat &image, int width, int height)
{
	int width_old = image.cols;
	int height_old = image.rows;
	double scale_width = static_cast<double>(width_old) / width;
	double scale_height = static_cast<double>(height_old) / height;
	double scale = std::max(scale_height, scale_width);

	int width_new = static_cast<int>(static_cast<double>(width_old) / scale);
	int height_new = static_cast<int>(static_cast<double>(height_old) / scale);

	int dw = width - width_new;
	int dh = height - height_new;
	int top = dh / 2;
	int bottom = dh - top;
	int left = dw / 2;
	int right = dw - left;


	cv::resize(image, image, cv::Size(width_new, height_new), 0, 0, cv::INTER_LINEAR);
	cv::copyMakeBorder(image, image, top, bottom, left, right, cv::BORDER_CONSTANT, 200.0);
	return {static_cast<double>(left), static_cast<double>(top), scale};
}


cv::Mat getPaddedROI(cv::Mat& image, std::vector<double>& bbox){
	cv::Rect roi(
			cv::Point{
					static_cast<int>(bbox[0]),
					static_cast<int>(bbox[1])
			},
			cv::Point{
					static_cast<int>(bbox[2]),
					static_cast<int>(bbox[3])
			});

	// Create rects representing the image and the ROI
	auto image_rect = cv::Rect({}, image.size());

	// Find intersection, i.e. valid crop region
	auto intersection = image_rect & roi;

	// Move intersection to the result coordinate space
	auto inter_roi = intersection - roi.tl();

	// Create black image and copy intersection
	cv::Mat crop = cv::Mat::zeros(roi.size(), image.type());
	image(intersection).copyTo(crop(inter_roi));
	return crop;
}

