#ifndef AGEESTIMATION_H
#define AGEESTIMATION_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class AgeEstimationModule : public BaseEstimationModule
{
public:
	AgeEstimationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<AgeEstimationModule>;
};


}
}


#endif // AGEESTIMATION_H
