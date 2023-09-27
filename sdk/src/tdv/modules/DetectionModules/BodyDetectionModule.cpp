#include <tdv/modules/DetectionModules/BodyDetectionModule.h>

namespace tdv {

namespace modules {

BodyDetectionModule::BodyDetectionModule(const tdv::data::Context& config) : BaseDetection::BaseDetectionModule<BodyDetectionModule>(config)
{}

const std::string BodyDetectionModule::CLASS_NAME = "body";

}
}
