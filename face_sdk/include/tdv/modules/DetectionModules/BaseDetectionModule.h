#ifndef Yv5DETECTOR_H
#define Yv5DETECTOR_H

#include <new>
#include <tuple>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/ONNXModule.h>
#include <tdv/utils/rassert/RAssert.h>

namespace{

std::tuple<int, int, double> resizeWithPad(cv::Mat& image, int new_width, int new_height) {

	double scale;
	int dw(0), dh(0);

	if (static_cast<float>(new_height) / new_width > static_cast<float>(image.rows) / image.cols)
	{
		scale = static_cast<double>(image.cols) / new_width;
		dh = (image.cols * new_height) / new_width - image.rows;
	}
	else
	{
		scale = static_cast<double>(image.rows) / new_height;
		dw = (image.rows * new_width) / new_height - image.cols;
	}

	int top = dh / 2;
	int bottom = dh - top;
	int left = dw / 2;
	int right = dw - left;

	cv::Scalar val = (image.depth() == CV_8U) ? cv::Scalar(127) : cv::Scalar(0.5);

	cv::copyMakeBorder(image, image, top, bottom, left, right, cv::BORDER_CONSTANT, val);
	cv::resize(image, image, cv::Size(new_width, new_height), 0, 0, cv::INTER_LINEAR);
	return std::make_tuple(left, top, scale);
}

cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, bool needBGR=false, const int ddepth=CV_32F) {

	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);

	if (needBGR)
		cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

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

inline float area(const std::vector<float>& v) {
	if (v.size()!=4)
		return 0;
	long area = (v[2]-v[0])*(v[3]-v[1]);
	return (area > 0) ? area : 0;
}

inline std::vector<float> intersection(const std::vector<float>& v1, const std::vector<float>& v2) {

	if (!(v1.size()==4 && v2.size()==4 && (v1[0]<v1[2]) && (v1[1]<v1[3]) && (v2[0]<v2[2]) && (v2[1]<v2[3])))
		return {0, 0, 0, 0};

	float x1_max = (v1[0] < v2[0]) ? v2[0] : v1[0];
	float y1_max = (v1[1] < v2[1]) ? v2[1] : v1[1];
	float x2_min = (v1[2] < v2[2]) ? v1[2] : v2[2];
	float y2_min = (v1[3] < v2[3]) ? v1[3] : v2[3];

	if ((x1_max >= x2_min) || (y1_max >= y2_min)) {
		return {0, 0, 0, 0};
	}
	return {x1_max, y1_max, x2_min, y2_min};
}

template <typename T>
inline bool SortScorePairDescend(const std::pair<float, T>& pair1,
						  const std::pair<float, T>& pair2)
{
	return pair1.first > pair2.first;
}

void GetMaxScoreIndex(const std::vector<float>& scores, const float threshold, const int top_k,
					  std::vector<std::pair<float, int> >& score_index_vec)
{
	// Generate index score pairs.
	for (size_t i = 0; i < scores.size(); ++i)
	{
		if (scores[i] > threshold)
		{
			score_index_vec.push_back(std::make_pair(scores[i], i));
		}
	}

	// Sort the score pair according to the scores in descending order
	std::stable_sort(score_index_vec.begin(), score_index_vec.end(),
					 SortScorePairDescend<int>);

	// Keep top_k scores if needed.
	if (top_k > 0 && top_k < (int)score_index_vec.size())
	{
		score_index_vec.resize(top_k);
	}
}


void NMSFast_(const std::vector<std::vector<float>>& bboxes,
	  const std::vector<float>& scores, const float score_threshold,
	  const float nms_threshold,
	  std::vector<int>& indices, const float eta=1.f, const int top_k=0)
{
	RHAssert(0x000000, bboxes.size() == scores.size());

	// Get top_k scores (with corresponding indices).
	std::vector<std::pair<float, int> > score_index_vec;
	GetMaxScoreIndex(scores, score_threshold, top_k, score_index_vec);

	// Do nms.
	float adaptive_threshold = nms_threshold;
	indices.clear();
	for (size_t i = 0; i < score_index_vec.size(); ++i) {
		const int idx = score_index_vec[i].second;
		bool keep = true;
		for (int k = 0; k < (int)indices.size() && keep; ++k) {
			const int kept_idx = indices[k];
			float intArea = static_cast<float>(area(intersection(bboxes[idx], bboxes[kept_idx])));
			float unionArea = area(bboxes[idx]) + area(bboxes[kept_idx]) - intArea;
			float overlap = intArea / unionArea;
			keep = overlap <= adaptive_threshold;
		}
		if (keep)
			indices.push_back(idx);
		if (keep && eta < 1 && adaptive_threshold > 0.5) {
		  adaptive_threshold *= eta;
		}
	}
}

float clip_value(float x, float min, float max)
{
	x = (x < min) ? min : (x > max) ? max : x;
	return x;
}

}

namespace tdv {

namespace modules {

namespace BaseDetection {

template <typename Impl>
class BaseDetectionModule : public ONNXModule<Impl>
{
public:
	BaseDetectionModule(const tdv::data::Context& config);

private:
	friend class ONNXModule<Impl>;
	void virtual preprocess(tdv::data::Context& data) override;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<std::vector<float>> getOutputData(std::shared_ptr<uint8_t> buff) const;
	tdv::data::Context processOutputData(std::vector<std::vector<float>> predictions,
									  const tdv::data::Context& metadata);
	const double IOU_THRESH;
	const double CONF_THRESH;
	const bool RAW_OUTPUT;
protected:
	bool needBGR = false;
};


template<typename Impl>
BaseDetectionModule<Impl>::BaseDetectionModule(const tdv::data::Context& config) :
	ONNXModule<Impl>(config),
	IOU_THRESH(config.get<double>("iou_threshold", 0.5)),
	CONF_THRESH(config.get<double>("confidence_threshold", 0.5)),
	RAW_OUTPUT(config.get<bool>("raw_output", false))
	{}

template<typename Impl>
std::vector<std::vector<float>> BaseDetectionModule<Impl>::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = this->getOutputShapes();
	RHAssert2(0x7b64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_count{static_cast<size_t>(shapes.front()[1])}, predict_shape{static_cast<size_t>(shapes.front()[2])};

	RHAssert2(0x7b64809c, predict_shape == 6, "unsupported output shape");

	std::vector<std::vector<float>> bboxes;

	auto types = this->getOutputTypes();
	switch (types.front()) { // ONNXTensorElementDataType
	case 1:	// maps to c type float
		for(size_t i=0; i < predict_count; ++i)
		{
			float* blob_data = reinterpret_cast<float*>(buff.get()+sizeof(float)*i*predict_shape);
			bboxes.emplace_back(blob_data, blob_data + predict_shape);
		}
		break;
	default:
		throw	tdv::utils::rassert::tdv_error(0xed26ca12, "unsupported output type");
	}
	return bboxes;
}

template<typename Impl>
void BaseDetectionModule<Impl>::preprocess(tdv::data::Context& data) {
	Context& imageInput = data.at("image");

	cv::Mat image = tdv::data::bsmToCvMat(imageInput, true);

	RHAssert2(0x11113333, image.depth() == CV_8U || image.depth() == CV_32F, "only 8U and 32F image types are suported");

	const auto& shape = this->getInputShapes();
	const auto& INPUT_HEIGHT = shape.front()[2];
	const auto& INPUT_WIDTH = shape.front()[3];
	const auto& N_CHANNEL = shape.front()[1];

	auto offset = resizeWithPad(image, INPUT_WIDTH, INPUT_HEIGHT);
	size_t sizeInBytes = INPUT_WIDTH * INPUT_HEIGHT * N_CHANNEL * sizeof(float);

	unsigned char* input_ptr = static_cast<unsigned char*>(malloc(sizeInBytes));
	if(!input_ptr)
		throw std::bad_alloc();
	cv::Mat img_blob = blobFromImage(image, N_CHANNEL, needBGR);
	memcpy(input_ptr, img_blob.data, sizeInBytes);

	Context& inputData = data["objects@input"][0];
	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr, [](unsigned char* ptr){ free(ptr);});
	inputData["resize_offset"] = offset;
	inputData["image_shape"] = data.at("image").at("shape");
	return;
}

template<typename Impl>
void BaseDetectionModule<Impl>::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<std::vector<float>> bboxes = getOutputData(buffer);
		if(!RAW_OUTPUT)
		{
			data["objects"] = processOutputData(bboxes, data);
		}
		else
			data["body_boxes"] = std::move(bboxes);
	}
}

template<typename Impl>
tdv::data::Context BaseDetectionModule<Impl>::processOutputData(std::vector<std::vector<float>> predictions,
									  const tdv::data::Context& metadata)
{
	std::vector<std::vector<float>> localBoxes;
	std::vector<float> localConfidences;
	std::vector<int> indices;

	for (const auto& cur_pred : predictions)
	{
		float obj_conf = cur_pred[4]; // confidence
		if (obj_conf < CONF_THRESH)
			continue;

		// convert box from xywh to xyxy
		float xc = cur_pred[0];
		float yc = cur_pred[1];
		float w = cur_pred[2];
		float h = cur_pred[3];
		float x1 = xc - w / 2;
		float y1 = yc - h / 2;
		float x2 = xc + w / 2;
		float y2 = yc + h / 2;
		localBoxes.push_back({x1, y1, x2, y2});
		localConfidences.push_back(obj_conf);
	}

	NMSFast_(localBoxes, localConfidences, 0.0, IOU_THRESH, indices);
	const auto offset = metadata.at("objects@input")[0].at("resize_offset").get<std::tuple<int, int, double>>();
	const auto img_height = metadata.at("objects@input")[0].at("image_shape")[0].get<long>();
	const auto img_width = metadata.at("objects@input")[0].at("image_shape")[1].get<long>();

	const Context& imageInput = metadata.at("image");
	std::shared_ptr<cv::Mat> image = std::shared_ptr<cv::Mat>(new cv::Mat(tdv::data::bsmToCvMat(imageInput, true)));

	tdv::data::Context objects;
	for (size_t i = 0; i < indices.size(); i++) {
		tdv::data::Context object;
		object["id"] = static_cast<long>(i);
		object["class"] = Impl::CLASS_NAME;
		object["confidence"] = (double)localConfidences[indices[i]];
		std::vector<float>& lbox = localBoxes[indices[i]];
		// convert to original image normalized coordinates
		lbox[0] = (lbox[0] * std::get<2>(offset) - std::get<0>(offset)) / img_width;
		lbox[1] = (lbox[1] * std::get<2>(offset) - std::get<1>(offset)) / img_height;
		lbox[2] = (lbox[2] * std::get<2>(offset) - std::get<0>(offset)) / img_width;
		lbox[3] = (lbox[3] * std::get<2>(offset) - std::get<1>(offset)) / img_height;

		for(auto coord : localBoxes[indices[i]])
		{
			coord = clip_value(coord, 0.0, 1.0);
			object["bbox"].push_back(static_cast<double>(coord));
		}
		object["object@image"] = image;
		objects.push_back(std::move(object));
	}
	return objects;
}

}
}
}


#endif // Yv5DETECTOR_H
