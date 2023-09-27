#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <tdv/data/ContextUtils.h>
#include <tdv/modules/EyeOpenessEstimationModule.h>
#include <tdv/utils/rassert/RAssert.h>

namespace {
cv::Mat blobFromImage(cv::Mat& image, int nchannel = 3, const int ddepth=CV_32F) 
{
	cv::Mat output;
	if(image.channels() != 1)
		cv::cvtColor(image, image, cv::COLOR_RGB2GRAY);
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
	RHAssert2(0x11561385, nchannel == 1 && nch == 1, "Need 1 channel image (Gray)");

	for(int i=0; i < nchannel; i++)
	{
		cv::Mat mean;
		cv::Mat std;
		cv::meanStdDev(ch[i], mean, std);
		ch[i] = (ch[i] - mean.at<double>(0)) / std.at<double>(0);
	}

	return output;
}
}

namespace tdv {
namespace modules {

EyeOpenessEstimationModule::EyeOpenessEstimationModule(const tdv::data::Context& config):
	ONNXModule<EyeOpenessEstimationModule>(config),
	OPNS_THRESH(config.get<double>("openness_threshold", 0.5693))
{};

std::vector<float> EyeOpenessEstimationModule::getOutputData(std::shared_ptr<uint8_t> buff) const
{
	const auto& shapes = getOutputShapes();

	RHAssert2(0xcb64809c, shapes.front()[0] == 1, "batch output not supported yet");

	size_t predict_shape{static_cast<size_t>(shapes.front()[1])};
	float* blob_data = reinterpret_cast<float*>(buff.get());
	std::vector<float> result_predict{blob_data, blob_data + predict_shape};

	return result_predict;
}

void EyeOpenessEstimationModule::preprocess(tdv::data::Context& data)
{
	Context& firstInput = data["image"];
	cv::Mat image = tdv::data::bsmToCvMat(firstInput, true);

	RHAssert2(0x7a11d233,  image.depth() == CV_8U ||  image.depth() == CV_32F, "only 8U and 32F image types are suported");

	const auto& shape = getInputShapes();
	const auto& INPUT_SIZE = shape.front()[2];
	const auto& N_CHANNEL = shape.front()[3];

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

void EyeOpenessEstimationModule::process(tdv::data::Context& data){
	Context& input = data["image"];
	cv::Mat face = tdv::data::bsmToCvMat(input, true);

	const tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];
	cv::Point2f left_eye_point  = cv::Point2f(obj["keypoints"]["left_eye"]["proj"][0].get<double>() * face.size[1],
											  obj["keypoints"]["left_eye"]["proj"][1].get<double>() * face.size[0]);
	cv::Point2f right_eye_point = cv::Point2f(obj["keypoints"]["right_eye"]["proj"][0].get<double>() * face.size[1],
											  obj["keypoints"]["right_eye"]["proj"][1].get<double>() * face.size[0]);
	std::vector<cv::Mat> result = get_crops_of_eyes(face, left_eye_point, right_eye_point);
	std::vector<double> eyes_openess(2);
	eye_flag = 0;
	for (size_t i = 0; i < 2; i++)
	{
		Context& eyeCtx = data["image"];
		eyeCtx.clear();
		tdv::data::cvMatToBsm(eyeCtx, result[eye_flag]);
		/////////////////////////////////
		ONNXModule::operator ()(data);///
		/////////////////////////////////
		eye_flag = 1;
	}
}


void EyeOpenessEstimationModule::operator ()(tdv::data::Context& data)
{
	RHAssert2(0x824dc576, data.contains("objects"), "need objects");
	for(int i = 0; i < data["objects"].size(); i++){
		data["objects@current_id"] = i;
		this->process(data);
	}
	data.erase("objects@current_id");
}

void EyeOpenessEstimationModule::postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data)
{
	if(buffer)
	{
		std::vector<float> predict = getOutputData(buffer);
		tdv::data::Context& obj = data["objects"][data["objects@current_id"].get<int>()];
		std::string result_key = (eye_flag) ? "is_right_eye_open" : "is_left_eye_open";

		for(size_t i = 0; i < predict.size(); i++)
		{
			double res_openness = 1.0 - static_cast<double>(predict[i]);
			obj[result_key]["value"] = static_cast<bool>(OPNS_THRESH < res_openness);
			obj[result_key]["confidence"] = res_openness;
		}
	}
}

}
}

cv::Matx23f estimate_scaled_rigid_transform(const std::vector<cv::Point2f> src, const std::vector<cv::Point2f> dst, const int iterations_count = 10)
{
	RHAssert(0x7c2efc4a, src.size() == dst.size());
	RHAssert(0xfbf84958, src.size() >= 2);

	std::vector<float> weights(src.size(), 1.f);
	std::vector<float> temp(src.size());

	for(size_t iterations = 0; ; ++iterations)
	{
		// scaled rigid transform is just a matrix:
		//    a b c
		//   -b a d
		// with arbitrary a, b, c and d

		// here we will iteratively compute it, reweighting points pairs by they error from prev iteration

		float sa[4][4], sb[4], sr[4];

		std::memset(sa, 0, sizeof(sa));
		std::memset(sb, 0, sizeof(sb));
		std::memset(sr, 0, sizeof(sr));

		for(size_t i = 0; i < src.size(); ++i)
		{
			const cv::Point2f &a = src[i];
			const cv::Point2f &b = dst[i];
			const float w = weights[i];

			sa[0][0] += w * ( a.x * a.x  +  a.y * a.y );
			sa[0][2] += w * ( a.x );
			sa[0][3] += w * ( a.y );
			sa[1][0] += w * ( a.y * a.x  -  a.x * a.y );
			sa[2][2] += w * ( 1.f );

			sb[0] += w * ( a.x * b.x + a.y * b.y );
			sb[1] += w * ( a.y * b.x - a.x * b.y );
			sb[2] += w * ( b.x );
			sb[3] += w * ( b.y );
		}

		sa[1][1] = sa[0][0];
		sa[1][2] = sa[0][3];
		sa[1][3] = - sa[0][2];

		sa[2][0] = sa[0][2];
		sa[2][1] = sa[0][3];

		sa[3][0] = sa[0][3];
		sa[3][1] = -sa[0][2];
		sa[3][3] = sa[2][2];

		float sw[4];
		for(int i = 0; i < 4; ++i)
			sw[i] = 1.f / sqrt( std::abs<float>(sa[i][i]) + 1e-6 );


		for(int i = 0; i < 4; ++i)
		{
			sb[i] *= sw[i];

			for(int j = 0; j < 4; ++j)
				sa[i][j] *= sw[i] * sw[j];
		}

		const bool solved = cv::solve(
			cv::Mat_<float>(4, 4, (float*) sa),
			cv::Mat_<float>(4, 1, (float*) sb),
			cv::Mat_<float>(4, 1, (float*) sr),
			cv::DECOMP_SVD);

		for(int i = 0; i < 4; ++i)
			sr[i] *= sw[i];

		RHAssert(0xb412a730, solved);

		if(iterations >= iterations_count)
		{
			cv::Matx23f result;
			result(0, 0) = sr[0];
			result(0, 1) = sr[1];
			result(0, 2) = sr[2];
			result(1, 0) = -sr[1];
			result(1, 1) = sr[0];
			result(1, 2) = sr[3];
			return result;
		}

		for(size_t i = 0; i < src.size(); ++i)
		{
			const cv::Point2f &a = src[i];
			const cv::Point2f &b = dst[i];

			cv::Point2f c;
			c.x =   a.x * sr[0] + a.y * sr[1] + sr[2];
			c.y = - a.x * sr[1] + a.y * sr[0] + sr[3];

			weights[i] = cv::norm(b-c);
		}

		temp = weights;

		std::nth_element(
			temp.data(),
			temp.data() + temp.size() / 2,
			temp.data() + temp.size());

		const float nw = temp[temp.size() / 2];

		for(size_t i = 0; i < src.size(); ++i)
			weights[i] = 1.f / (nw + weights[i] + 1e-6);
	}
}

std::vector<cv::Mat> get_crops_of_eyes(cv::Mat face, cv::Point2f left_eye, cv::Point2f right_eye)
{
	std::vector<cv::Point2f> dst_points = {cv::Point2f(left_eye), cv::Point2f(right_eye)};
	std::vector<cv::Point2f> src_points = {
			cv::Point2f(68.f, 29.f),
			cv::Point2f(131.f, 29.f),
	};

	std::vector<cv::Mat> out;
	cv::Size dsize = cv::Size(200, 200);
	cv::Matx23f transform_m = estimate_scaled_rigid_transform(src_points, dst_points, 10);  // 10 = iterations_count
																															
	cv::warpAffine(
		face,
		face,
		transform_m,
		dsize,
		cv::WARP_INVERSE_MAP | cv::INTER_LINEAR,
		cv::BORDER_CONSTANT,
		cv::Scalar()
		);
	const int size = 53;
	cv::Rect left_eyeROI(40, 0, size, size);
	cv::Mat left_eye_crop = face(left_eyeROI);

	cv::Rect right_eyeROI(110, 0, size, size);
	cv::Mat right_eye_crop = face(right_eyeROI);

	out = {left_eye_crop, right_eye_crop};

	return out;
}
