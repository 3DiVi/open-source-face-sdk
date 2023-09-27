#include <new>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/BodyReidentificationModule.h>
#include <tdv/utils/rassert/RAssert.h>



namespace {


float* blobFromImage(cv::Mat& img){
	float* blob = new float[img.total()*3];
	int channels = 3;
	int img_h = img.rows;
	int img_w = img.cols;

	std::vector<float> mean = {0.485, 0.456, 0.406};
	std::vector<float> std = {0.229, 0.224, 0.225};

	for (size_t c = 0; c < channels; c++)
	{
		for (size_t  h = 0; h < img_h; h++)
		{
			for (size_t w = 0; w < img_w; w++)
			{
				blob[c * img_w * img_h + h * img_w + w] =
							(((float)img.at<cv::Vec3b>(h, w)[c]) / 255.0f - mean[c]) / std[c];
			}
		}
	}
	return blob;
}


}



namespace tdv {

namespace modules {


BodyReidentificationModule::BodyReidentificationModule(const tdv::data::Context& config) :
		ONNXModule<BodyReidentificationModule>(config)
{};

std::vector<float> BodyReidentificationModule::getOutputData(std::shared_ptr<uint8_t> buff)
{
	const auto& shapes = getOutputShapes();
	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());

	return std::vector<float>(blob_data, blob_data + predict_shape);
}

void BodyReidentificationModule::preprocess(tdv::data::Context& data) {

	Context& imageInput = data.at("image");
	cv::Mat image = tdv::data::bsmToCvMat(imageInput, true);

	const auto& shapes = getInputShapes();
	const int64_t& INPUT_H = shapes[0][2];
	const int64_t& INPUT_W = shapes[0][3];
	cv::Mat pr_img;
	cv::resize(image, pr_img, cv::Size(INPUT_W, INPUT_H));
	float* blob = blobFromImage(pr_img);
	Context inputTensor;
	inputTensor["input_ptr"] = std::shared_ptr<unsigned char>((unsigned char*)blob, [](unsigned char* ptr){ free(ptr);});
	data["objects@input"].push_back(inputTensor);

	return;
}

void BodyReidentificationModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) {
	if(buffer)
	{
		std::vector<float> embeds = getOutputData(buffer);
		tdv::data::Context& output_data = data["output_data"];
		tdv::data::Context& templateData = output_data["template"];

		output_data["template_size"] = embeds.size();

		for (double value : embeds)
		{
			templateData.push_back(value);
		}
	}
}

}
}
