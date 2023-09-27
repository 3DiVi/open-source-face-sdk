#include <tdv/modules/AgeEstimationModule.h>
#include <tdv/inference/AgeEstimationInference.h>


namespace tdv {

namespace modules {


AgeEstimationModule::AgeEstimationModule(const tdv::data::Context& config):
	BaseEstimationModule(config)
{

	this->module_version = config.get<int64_t>("model_version", 1);
	if (module_version == 1)
		block = std::unique_ptr<ProcessingBlock>(new tdv::modules::AgeEstimationInference<AgeEstimationModule>(config));
	else if(module_version == 2)
		block = std::unique_ptr<ProcessingBlock>(new tdv::modules::AgeEstimationInference<AgeEstimationModule, KEYPOINTS_BASED_CROP>(config));
	else
		throw std::runtime_error("not support version");
}

}
}
