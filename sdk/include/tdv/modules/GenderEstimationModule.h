#ifndef GENDERNEWESTIMATION_H
#define GENDERNEWESTIMATION_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class GenderEstimationModule : public BaseEstimationModule
{
public:
	GenderEstimationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<GenderEstimationModule>;
};


}
}


#endif // GENDERNEWESTIMATION_H
