#ifndef BODYDETECTOR_H
#define BODYDETECTOR_H

#include <tdv/modules/DetectionModules/BaseDetectionModule.h>

namespace tdv {

namespace modules {

class BodyDetectionModule : public BaseDetection::BaseDetectionModule<BodyDetectionModule> {
	static void getEncriptionKey(int64_t &key_data_len, unsigned char const* &key_data, int model_version=1);
	friend class ONNXModule<BodyDetectionModule>;

public:
	BodyDetectionModule(const tdv::data::Context& config);
	static const std::string CLASS_NAME;
};

}
}


#endif // BODYDETECTOR_H
