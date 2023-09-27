#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/LivenessDetectionModule/LivenessDetectionModule.h>
#include <tdv/modules/LivenessDetectionModule/LivenessBaseModule.h>
#include <tdv/utils/rassert/RAssert.h>
#include <math.h>
#include <opencv2/core.hpp>

namespace {
cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) 
{
	cv::Mat output;
	if(image.channels() == 1)
		cv::cvtColor(image, image, cv::COLOR_GRAY2RGB);
	image.convertTo(image, CV_32FC3);

	int nch = image.channels();
	int sz[] = { 1, image.rows, image.cols, nchannel};

	output.create(4, sz, ddepth);

	std::vector<cv::Mat> ch(nch);
	
	for( int j = 0; j < nchannel; j++ )
		ch[j] = cv::Mat(image.rows, image.cols, ddepth, output.ptr(0,j));
	cv::split(image, ch);

	RHAssert2(0x11561385, nchannel == 3 && nch == 3, "Need 3 channel image (RGB)");
	return output;
}

cv::Rect getRectScale(const cv::Rect detection, const cv::Size image_size, const float scale)
{
	cv::Rect optimal_rect(detection);
	float current_scale = 1.0;
	bool correct = true;
	while (correct)
	{
		if (current_scale >= scale)
			break;

		current_scale += 0.1;
		int new_width = (int) detection.width * current_scale;
		int new_height = (int) detection.height * current_scale;
		int new_x = (int) detection.x - (new_width - detection.width) / 2;
		int new_y = (int) detection.y - (new_height - detection.height) / 2;

		correct = (new_x >= 0) && (new_y >= 0) &&
				  (new_x + new_width < image_size.width) && (new_y + new_height < image_size.height);

		if (correct)
			optimal_rect = cv::Rect(new_x, new_y, new_width, new_height);
	}
	return optimal_rect;
}


template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max<T>(lower, std::min<T>(n, upper));
}

}

namespace tdv {
namespace modules {

LivenessDetectionModule::LivenessDetectionModule(const tdv::data::Context& config):
			LIVENESS_THRESH(config.get<double>("liveness_threshold", 0.9))
{
	Context LivenessModel2_7Ctx;
	LivenessModel2_7Ctx["model_path"] = config["model_scale2.7_path"];
	LivenessModel2_7Ctx["model_version"] = 1l;

	Context LivenessModel4_0Ctx;
	LivenessModel4_0Ctx["model_path"] = config["model_scale4.0_path"];
	LivenessModel4_0Ctx["model_version"] = 2l;
	

	model1 = std::make_shared<tdv::modules::LivenessBaseModule> (LivenessModel2_7Ctx);
	model2 = std::make_shared<tdv::modules::LivenessBaseModule> (LivenessModel4_0Ctx);
}

void LivenessDetectionModule::operator ()(tdv::data::Context& data)
{
	RHAssert2(0xaa59e948, data.contains("objects"), "need objects");
	for(int i = 0; i < data["objects"].size(); i++){
		data["objects@current_id"] = i;
		this->process(data);
	}
	data.erase("objects@current_id");
}


void LivenessDetectionModule::process(tdv::data::Context& data)
{
	tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];
	if(obj.get<std::string>("class", "").compare("face"))
	{
		RHAssert2(0x7a11d253,  false, "input is not a face!");
	}
	tdv::data::Context& firstInput = data["image"];
	cv::Mat input = tdv::data::bsmToCvMat(firstInput, true);

	cv::cvtColor(input, input, cv::COLOR_BGR2RGB);

	const Context& rectCtx = obj["bbox"];  // const overload calls .at()

	int face_bbox_top_left_x =     clip(static_cast<int>(rectCtx[0].get<double>() * input.cols), 0, input.cols);
	int face_bbox_top_left_y =     clip(static_cast<int>(rectCtx[1].get<double>() * input.rows), 0 , input.rows);
	int face_bbox_bottom_rigth_x = clip(static_cast<int>(rectCtx[2].get<double>() * input.cols), 0, input.cols);
	int face_bbox_bottom_rigth_y = clip(static_cast<int>(rectCtx[3].get<double>() * input.rows), 0 , input.rows);

	// TODO: Below is an EXAMPLE of getting a face crop. Now face_detector returns a rectangular bbox, but we need a square bbox to work correctly.
	//we need a square bbox, but rectangle given, so:
	int correction_element = ((face_bbox_bottom_rigth_y - face_bbox_top_left_y) - (face_bbox_bottom_rigth_x - face_bbox_top_left_x)) / 2;
	int bbox_top_left_x = face_bbox_top_left_x;
	int bbox_top_left_y = face_bbox_top_left_y + correction_element;
	int side_length = face_bbox_bottom_rigth_x - face_bbox_top_left_x;

	cv::Rect detection = cv::Rect(bbox_top_left_x, bbox_top_left_y, side_length, side_length);
	cv::Mat image = input(detection);

	const cv::Rect optimal_rect1 = getRectScale(detection, input.size(), scales[0]);
	const cv::Rect optimal_rect2 = getRectScale(detection, input.size(), scales[1]);

	cv::Mat crop1, crop2;
	cv::resize(input(optimal_rect1), crop1, cv::Size(80, 80));
	cv::resize(input(optimal_rect2), crop2, cv::Size(80, 80));

	if (input.channels() == 1  ) {
		cv::cvtColor(crop1, crop1, cv::COLOR_GRAY2RGB);
		cv::cvtColor(crop2, crop2, cv::COLOR_GRAY2RGB);
	}

	crop1.convertTo(crop1, CV_32FC3);
	crop2.convertTo(crop2, CV_32FC3);

	tdv::data::Context livenessIoData1;
	tdv::data::Context& livenessImg1Context = livenessIoData1["image"];
	tdv::data::cvMatToBsm(livenessImg1Context, crop1);

	tdv::data::Context livenessIoData2;
	tdv::data::Context& livenessImg2Context = livenessIoData2["image"];
	tdv::data::cvMatToBsm(livenessImg2Context, crop2);

//////////////////////////////////////
	(*model1)(livenessIoData1);///////
	(*model2)(livenessIoData2);///////
//////////////////////////////////////

	float predict1;
	float predict2;
	for(const Context& obj : livenessIoData1["objects"])
	{
		predict1 = obj["liveness"].get<double>();
	}
	for(const Context& obj : livenessIoData2["objects"])
	{
		predict2 = obj["liveness"].get<double>();
	}

	tdv::data::Context& liveness = obj["liveness"];
	liveness["confidence"] = static_cast<double>(predict1 + predict2) / 2.0f;

	if (((predict1 + predict2) / 2.0f) > LIVENESS_THRESH)
		liveness["value"] = static_cast<std::string>("REAL");
	else
		liveness["value"] = static_cast<std::string>("FAKE");
}

}
}