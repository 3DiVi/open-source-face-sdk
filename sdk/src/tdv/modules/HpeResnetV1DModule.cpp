#include <new>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/HpeResnetV1DModule.h>
#include <tdv/utils/rassert/RAssert.h>
#include <tdv/utils/har_utils/har_utils.h>


namespace {

cv::Mat imageToBlob(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) {

	cv::Mat output;

	if(image.depth() == CV_8U && ddepth == CV_32F)
	{
		image.convertTo(image, ddepth, 1.f/255);
	}

	int nch = image.channels();
	int sz[] = { 1, nchannel, image.rows, image.cols};

	output.create(4, sz, ddepth);

	std::vector<cv::Mat> ch(nch);
	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));

	cv::split(image, ch);

	//Normalize imag
	const double mean[] = {0.485, 0.456, 0.406};
	const double std[] = {0.229, 0.224, 0.225};
	for(int i=0; i < nchannel; i++)
		ch[i] = (ch[i] - mean[i]) / std[i];

	return output;
}


}


namespace tdv {
namespace modules {

HpeResnetV1DModule::HpeResnetV1DModule(const tdv::data::Context &config) :
	ONNXModule<HpeResnetV1DModule>(config),
	RAW_OUTPUT(config.get<bool>("raw_output", false)),
	LABEL_MAP(read_label_map(config.at("label_map").get<std::string>()))
	{}


void HpeResnetV1DModule::preprocess(tdv::data::Context &data)
{
	const auto& shapes = getInputShapes();
	const auto& INPUT_HEIGHT = shapes.front()[2];
	const auto& INPUT_WIDTH = shapes.front()[3];
	const auto& N_CHANNEL = shapes.front()[1];

	cv::Mat image = tdv::data::bsmToCvMat(data["image"], true);

	RHAssert2(0x11113333, image.depth() == CV_8U || image.depth() == CV_32F, "only 8U and 32F image types are suported");

	if (image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);

	Context &inputData = data["objects@input"][0];
	inputData.clear();

	size_t sizeInBytesOnePerson = INPUT_WIDTH * INPUT_HEIGHT * N_CHANNEL * sizeof(float);
	unsigned char *input_ptr = static_cast<unsigned char *>(malloc(
			sizeInBytesOnePerson * data["objects"].size()));
	if (!input_ptr)
		throw std::bad_alloc();
	for (size_t i = 0; i < data["objects"].size(); ++i)
	{
		const Context &obj = data["objects"][i];
		const Context &rectCtx = obj["bbox"];
		std::vector<double> bbox = {
				rectCtx[0].get<double>() * image.size[1],
				rectCtx[1].get<double>() * image.size[0],
				rectCtx[2].get<double>() * image.size[1],
				rectCtx[3].get<double>() * image.size[0]
		};

		bboxScaler(bbox, {192, 256}, 1.25);
		cv::Mat roi = getPaddedROI(image, bbox);
		Context offset;
		for (auto offset_item : resizeWithPad(roi, INPUT_WIDTH, INPUT_HEIGHT)){
			offset.push_back(offset_item);
		}
		offset.push_back(bbox[0]);
		offset.push_back(bbox[1]);
		inputData["result_offset"].push_back(std::move(offset));

		cv::cvtColor(roi, roi, cv::COLOR_BGR2RGB);
		cv::Mat blob = imageToBlob(roi);
		memcpy(input_ptr + i * sizeInBytesOnePerson, blob.data, sizeInBytesOnePerson);
		inputData["id"].push_back(obj["id"]);

	}

	inputData["input_ptr"] = std::shared_ptr<unsigned char>(input_ptr,
															[](unsigned char *ptr) { free(ptr); });
	inputData["batch_size"] = data["objects"].size();
}

void HpeResnetV1DModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context &data)
{
	if (buffer)
	{
		const Context &meta = data["objects@input"][0];
		std::map<int, std::vector<std::vector<float>>> keypoints = getOutputData(buffer, meta);
		if (!RAW_OUTPUT)
		{
			std::vector<int> dims;
			for (const auto &dim: data["image"]["shape"])
				dims.push_back(static_cast<int>(dim.get<int64_t>()));
			for (Context &obj: data["objects"])
			{
				int id = obj["id"].get<int64_t>();
				obj["keypoints"] = pose_vector2normalizedCtx(keypoints[id], LABEL_MAP, dims);
			}
		}
	}
}

std::map<int, std::vector<std::vector<float>>>
HpeResnetV1DModule::getOutputData(std::shared_ptr<uint8_t> buff, const tdv::data::Context &meta)
{
	size_t predict_heatmap{17}, predict_width{64}, predict_height{48};
	std::map<int, std::vector<std::vector<float>>> keypoints;
	for (size_t p = 0; p < meta["id"].size(); ++p)
	{
		int id = meta["id"][p].get<int64_t>();
		auto &offset = meta["result_offset"][p];
		auto heatmapShiftX = offset[0].get<double>();
		auto heatmapShiftY = offset[1].get<double>();
		auto scaleX = offset[2].get<double>();
		auto scaleY = offset[2].get<double>();
		auto shiftX = offset[3].get<double>();
		auto shiftY = offset[4].get<double>();

		keypoints[id] = std::vector<std::vector<float>>(predict_heatmap, std::vector<float>(3, 0.0));
		for (size_t h = 0; h < predict_heatmap; ++h)
		{
			float *blob_data = reinterpret_cast<float *>(buff.get() + sizeof(float) *
																	  ((p * predict_heatmap + h) *
																	   predict_width * predict_height));
			cv::Mat heatmap(predict_width, predict_height, CV_32F, blob_data);
			double minVal;
			double maxVal;
			cv::Point minLoc;
			cv::Point maxLoc;
			minMaxLoc(heatmap, &minVal, &maxVal, &minLoc, &maxLoc);
			keypoints[id][h][0] = (4 * maxLoc.x - heatmapShiftX) * scaleX + shiftX;
			keypoints[id][h][1] = (4 * maxLoc.y - heatmapShiftY) * scaleY + shiftY;
			keypoints[id][h][2] = maxVal;
		}
	}
	return keypoints;
}

}
}
