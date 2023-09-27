#include <tdv/modules/EmotionsEstimationModule.h>
#include <tdv/inference/EmotionsEstimationInference.h>


namespace tdv {

namespace modules {


EmotionsEstimationModule::EmotionsEstimationModule(const tdv::data::Context& config):
	BaseEstimationModule(config)
{
	block = std::unique_ptr<ProcessingBlock>(new tdv::modules::EmotionsEstimationInference<EmotionsEstimationModule>(config));
}

}
}
