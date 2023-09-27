#ifndef GLASSESSESTIMATOR_H
#define GLASSESSESTIMATOR_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class GlassesEstimationModule : public BaseEstimationModule
{
public:
	GlassesEstimationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<GlassesEstimationModule>;
};


}
}


#endif // GLASSESSESTIMATOR_H
