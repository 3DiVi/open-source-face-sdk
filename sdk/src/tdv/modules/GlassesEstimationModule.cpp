#include <tdv/modules/GlassesEstimationModule.h>
#include <tdv/inference/GlassesEstimationInference.h>


namespace tdv {

namespace modules {


GlassesEstimationModule::GlassesEstimationModule(const tdv::data::Context& config):
	BaseEstimationModule(config)
{
	block = std::unique_ptr<ProcessingBlock>(new tdv::modules::GlassesEstimationInference<GlassesEstimationModule>(config));
}

}
}
