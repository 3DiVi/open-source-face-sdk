#ifndef MASKESTIMATION_H
#define MASKESTIMATION_H

#include <tdv/modules/ONNXModule.h>
#include <tdv/modules/BaseEstimationModule.h>

namespace tdv {

namespace modules {

class MaskEstimationModule : public BaseEstimationModule
{
public:
	MaskEstimationModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<MaskEstimationModule>;
	static void getEncriptionKey(int64_t &key_data_len, unsigned char const* &key_data, int model_version=1);
};


}
}


#endif // MASKESTIMATION_H
