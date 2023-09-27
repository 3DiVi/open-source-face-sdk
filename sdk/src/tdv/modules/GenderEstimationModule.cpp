#include <tdv/modules/GenderEstimationModule.h>
#include <tdv/inference/GenderEstimationInference.h>


namespace tdv {

namespace modules {


GenderEstimationModule::GenderEstimationModule(const tdv::data::Context& config):
	BaseEstimationModule(config)
{
	this->module_version = config.get<int64_t>("model_version", 1);
	if (module_version == 1)
		block = std::unique_ptr<ProcessingBlock>(new tdv::modules::GenderEstimationInference<GenderEstimationModule>(config));
	else if(module_version == 2)
		block = std::unique_ptr<ProcessingBlock>(new tdv::modules::GenderEstimationInference<GenderEstimationModule, KEYPOINTS_BASED_CROP>(config));
	else
		throw std::runtime_error("not support version");
}

}
}
