#ifndef LIVENESSBASE_H
#define LIVENESSBASE_H

#include <tdv/modules/ONNXModule.h>

namespace tdv {

namespace modules {

class LivenessBaseModule : public ONNXModule<LivenessBaseModule>
{
public:
	LivenessBaseModule(const tdv::data::Context& config);
private:
	friend class ONNXModule<LivenessBaseModule>;
	void virtual preprocess(tdv::data::Context& data) override;
	void virtual postprocess(std::shared_ptr<uint8_t> buffer, tdv::data::Context& data) override;
	std::vector<float> getOutputData(std::shared_ptr<uint8_t> buff) const;


};

}
}

#endif