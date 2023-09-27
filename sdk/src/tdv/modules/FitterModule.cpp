#include <tdv/modules/FitterModule.h>
#include <tdv/inference/MeshFitterInference.h>


namespace tdv {

namespace modules {


FitterModule::FitterModule(const tdv::data::Context& config):
	BaseEstimationModule(config)
{
	this->module_version = config.get<int64_t>("model_version", 1);
	if (module_version == 1)
		block = std::unique_ptr<ProcessingBlock>(new tdv::modules::MeshFitterInference<FitterModule>(config));
	else
		throw std::runtime_error("not support version");
}

}
}
