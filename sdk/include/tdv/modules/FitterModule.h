#ifndef MESHFITTER_H
#define MESHFITTER_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class FitterModule : public BaseEstimationModule
{
public:
	FitterModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<FitterModule>;
};

}
}


#endif // MESHFITTER_H
