#include <tdv/modules/DetectionModules/FaceDetectionModule.h>

namespace tdv {

namespace modules {

FaceDetectionModule::FaceDetectionModule(const tdv::data::Context& config) : BaseDetection::BaseDetectionModule<FaceDetectionModule>(config)
{
	this->needBGR = true;
}

const std::string FaceDetectionModule::CLASS_NAME = "face";

}
}
