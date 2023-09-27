#ifndef TDV_HAR_UTILS_H
#define TDV_HAR_UTILS_H


cv::Mat blobFromImage(cv::Mat &image, int nchannel = 3, const int ddepth = CV_32F);

std::map<int, std::string> read_label_map(std::string label_path);

tdv::data::Context pose_vector2normalizedCtx(std::vector<std::vector<float>> &keypoints, std::map<int,
		std::string> &label_map, std::vector<int> &dims);

void bboxScaler(std::vector<double> &bbox, const std::vector<int> &dims, double padding);

std::vector<double> resizeWithPad(cv::Mat &image, int width, int height);

cv::Mat getPaddedROI(cv::Mat& image, std::vector<double>& bbox);
#endif
